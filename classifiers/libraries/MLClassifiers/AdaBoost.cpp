/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Charles Dubout (charles.dubout@idiap.ch)
*
* This file is part of the MASH Framework.
*
* The MASH Framework is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License version 2 or
* the GNU General Public License version 3 as published by the Free
* Software Foundation, whichever suits the most your needs.
*
* The MASH Framework is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public Licenses
* along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/


//------------------------------------------------------------------------------
/// @file MLClassifiers/AdaBoost.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.02.09
/// @version 2.2
//------------------------------------------------------------------------------

#include "AdaBoost.h"
#include "Utils.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

using namespace ML;

AdaBoost::AdaBoost(unsigned int nbRounds,
				   unsigned int nbFeaturesPerHeuristic,
				   unsigned int maxDepth,
				   double tca)
: nbRounds_(nbRounds), nbFeaturesPerHeuristic_(nbFeaturesPerHeuristic),
  maxDepth_(maxDepth), tca_(tca) {
	// The number of rounds must be strictly positive
	assert(nbRounds);

	// The TCA threshold must be positive
	assert(tca >= 0.0);
}

Classifier* AdaBoost::clone() const {
	return new AdaBoost(*this);
}

//------------------------------------------------------------------------------
/// @brief	AdaBoost loss, as derived in "A Theory of Multiclass Boosting" paper
//------------------------------------------------------------------------------
class AdaBoostLoss {
public:
	AdaBoostLoss(const InputSet& inputSet,
				 const std::vector<unsigned int>& predictions,
				 const std::vector<std::vector<double> >& costs);

	scalar_t operator()(scalar_t alpha) const;

private:
	const InputSet& inputSet_;
	const std::vector<unsigned int>& predictions_;
	const std::vector<std::vector<double> >& costs_;
};

AdaBoostLoss::AdaBoostLoss(const InputSet& inputSet,
						   const std::vector<unsigned int>& predictions,
						   const std::vector<std::vector<double> >& costs)
: inputSet_(inputSet), predictions_(predictions), costs_(costs) {
	// Nothing to do
}

scalar_t AdaBoostLoss::operator()(scalar_t alpha) const {
	// Get the number of samples and labels
	const unsigned int nbSamples = inputSet_.nbSamples();
	const unsigned int nbLabels = inputSet_.nbLabels();

	// Get the true labels of the samples
	const unsigned int* labels = inputSet_.labels();

	// Compute AdaBoost loss
	double loss = 0.0;

	const double expAlpha = std::exp(alpha);
	const double expMinusAlpha = 1.0 / expAlpha;

	for(unsigned int s = 0; s < nbSamples; ++s) {
		double alpha = (predictions_[s] == labels[s]) ? expMinusAlpha : 1.0;

		for(unsigned int l = 0; l < nbLabels; ++l) {
			if(l != labels[s]) {
				double beta = (predictions_[s] == l) ? expAlpha : 1.0;
				loss += costs_[s][l] * alpha * beta;
			}
		}
	}

	return loss / nbSamples;
}

void AdaBoost::train(InputSet& inputSet) {
	// Delete all the previous weak learners
	weakLearners_.clear();
	alphas_.clear();
	reported_.clear();

	// Get the number of samples, features, heuristics, and labels
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbHeuristics = inputSet.nbHeuristics();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Get the heuristic associated to every feature
	const unsigned int* heuristics = inputSet.heuristics();

	// Get the labels associated to every samples
	const unsigned int* labels = inputSet.labels();

	// Separate the features by heuristic
	std::vector<std::vector<unsigned int> > features(nbHeuristics);

	for(unsigned int f = 0; f < nbFeatures; ++f) {
		features[heuristics[f]].push_back(f);
	}

	std::cout << "[AdaBoost::train] #samples: " << nbSamples << ", #features: "
			  << nbFeatures << ", #heuristics: " << nbHeuristics
			  << ", #labels: " << nbLabels;

	// The total number of features to sample per round
	unsigned int nbFeaturesPerRound = 0;

	// The number of rounds that can fit in the cache
	unsigned int nbCachedRounds = 0;

	if(nbFeaturesPerHeuristic_) {
		for(unsigned int h = 0; h < nbHeuristics; ++h) {
			nbFeaturesPerRound += std::min(nbFeaturesPerHeuristic_,
										   static_cast<unsigned int>(features[h].size()));
		}

		// Make sure that the cache is big enough to accommodate at least one
		// boosting round
		assert(nbSamples * nbFeaturesPerRound <= NB_FEATURES_MAX);

		std::cout << ", #features/round: " << nbFeaturesPerRound;

		// Is it better to cache some rounds or all the dataset?
		if(nbSamples * nbFeatures <= NB_FEATURES_MAX &&
		   nbSamples * nbFeatures <= nbSamples * nbFeaturesPerRound * nbRounds_) {
			// Cache the whole input-set
			std::cout << '.' << std::endl;
			inputSet.samples();
			nbCachedRounds = 1;
		}
		else {
			nbCachedRounds = std::min(NB_FEATURES_MAX /
									  (nbSamples * nbFeaturesPerRound), nbRounds_);

			std::cout << ", #rounds/cache: " << nbCachedRounds << '.' << std::endl;
		}
	}
	else {
		assert(nbSamples * nbFeatures <= NB_FEATURES_MAX);
	}

	// Costs matrix
	std::vector<std::vector<double> > costs(nbSamples,
											std::vector<double>(nbLabels, 1.0));

	for(unsigned int s = 0; s < nbSamples; ++s) {
		costs[s][labels[s]] = 1.0 - nbLabels;
	}

	// Hypotheses matrix
	std::vector<std::vector<double> > hypotheses(nbSamples,
											  	 std::vector<double>(nbLabels));

	// Prediction matrix (responses of the current weak learner)
	std::vector<unsigned int> predictions(nbSamples);

	// Previous predictions
	std::vector<std::vector<unsigned int> > oldPredictions;

	// Flag indicating that there is features in the cache
	bool cached = false;

	// If we need to sample features
	std::vector<unsigned int> indices;

	// Do nbRounds rounds of boosting
	for(unsigned int r = 0; r < nbRounds_; ++r) {
		if(nbFeaturesPerHeuristic_) {
			unsigned int i = r % nbCachedRounds;

			if(i == 0) {
				if(cached) {
					inputSet.popFeatures();
					indices.clear();
				}

				for(unsigned int j = 0; j < std::min(nbCachedRounds, nbRounds_ - r); ++j) {
					for(unsigned int h = 0; h < nbHeuristics; ++h) {
						if(features[h].size() > nbFeaturesPerHeuristic_) {
							std::random_shuffle(features[h].begin(), features[h].end());
							indices.insert(indices.end(), features[h].begin(),
										   features[h].begin() + nbFeaturesPerHeuristic_);
						}
						else {
							indices.insert(indices.end(), features[h].begin(),
										   features[h].end());
						}
					}
				}

				inputSet.pushFeatures(indices);
				inputSet.features();
				cached = true;
			}
			else if(indices.size() > nbFeaturesPerRound) {
				std::copy_backward(indices.begin() + nbFeaturesPerRound,
								   indices.end(),
								   indices.end() - nbFeaturesPerRound);
				indices.resize(indices.size() - nbFeaturesPerRound);
			}

			if(nbCachedRounds > 1) {
				std::vector<unsigned int> indexes(nbFeaturesPerRound);

				for(unsigned int j = 0; j < nbFeaturesPerRound; ++j) {
					indexes[j] = i * nbFeaturesPerRound + j;
				}

				inputSet.pushFeatures(indexes);
			}
		}

		// Train a new weak learner on the current costs matrix
		WeakTree weakLearner(maxDepth_);
		weakLearner.train(inputSet, costs);

		// Save the predictions of the weak learner
		for(unsigned int s = 0; s < nbSamples; ++s) {
			predictions[s] = weakLearner.classify(inputSet, s);
		}

		// If we sampled features
		if(nbFeaturesPerHeuristic_) {
			weakLearner.remap(indices);

			if(nbCachedRounds > 1) {
				inputSet.popFeatures();
			}
		}

		// Find the alpha minimizing the loss
		AdaBoostLoss ABloss(inputSet, predictions, costs);
		double alpha = Utils::lineSearch(0.0, 10.0, ABloss);
		double edge = (std::exp(2.0 * alpha) - 1.0) / (std::exp(2.0 * alpha) + 1.0);

		if(edge < 1e-6) {
			std::cerr << "[AdaBoost::train] Couldn't find a weak learner."
					  << std::endl;
			break;
		}
		else if(Utils::geq(edge, 1.0)) {
			std::cout << "[AdaBoost::train] (Nearly) Perfect weak learner found!"
					  << std::endl;
			weakLearners_.push_back(weakLearner);
			alphas_.push_back(1.0);
			break;
		}

		weakLearners_.push_back(weakLearner);
		alphas_.push_back(alpha);

		if(tca_) {
			oldPredictions.push_back(predictions);
		}

		// Update the hypotheses and compute the training error
		double trainingError = 0;

		for(unsigned int s = 0; s < nbSamples; ++s) {
			hypotheses[s][predictions[s]] += alpha;

			double sum = 0.0;

			for(unsigned int l = 0; l < nbLabels; ++l) {
				if(l != labels[s]) {
					costs[s][l] = std::exp(hypotheses[s][l] -
										   hypotheses[s][labels[s]]);

					sum += costs[s][l];
				}
			}

			costs[s][labels[s]] = -sum;

			unsigned int predicted = std::max_element(hypotheses[s].begin(),
													  hypotheses[s].end()) -
									 hypotheses[s].begin();

			if(predicted != labels[s]) {
				trainingError += 1.0 / nbSamples;
			}
		}

		std::cout.precision(4);

		// Current loss
		double loss = ABloss(0.0);

		std::cout << "[AdaBoost::train] Round: " << (r + 1) << '/'
				  << nbRounds_ << ", edge: " << edge << ", alpha: " << alpha
				  << ", loss: " << loss << ", training error: "
				  << trainingError << "." << std::endl;

		// Totally Corrected Step (TCA)
		for(unsigned int w = 0, i = 0, z = 10 * (r + 1); tca_ && i <= r && z;
			++w, ++i, w %= r + 1, --z) {
			if(alphas_[w] > tca_) {
				AdaBoostLoss ABloss(inputSet, oldPredictions[w], costs);
				alpha = Utils::lineSearch(-1.0, 1.0, ABloss, tca_);

				if(std::abs(alpha) >= tca_) {
					// Update the weight of the weak learner
					alphas_[w] += alpha;

					// Update the weights of the samples
					for(unsigned int s = 0; s < nbSamples; ++s) {
						hypotheses[s][oldPredictions[w][s]] += alpha;

						double sum = 0.0;

						for(unsigned int l = 0; l < nbLabels; ++l) {
							if(l != labels[s]) {
								costs[s][l] = std::exp(hypotheses[s][l] -
													   hypotheses[s][labels[s]]);

								sum += costs[s][l];
							}
						}

						costs[s][labels[s]] = -sum;
					}

					std::cout << "[AdaBoost::train] TCA, alpha[" << w << "] += "
							  << alpha << " (" << alphas_[w] << "), loss: "
							  << ABloss(0.0) << '.' << std::endl;

					i = 0;
				}
			}
		}
	}

	if(cached) {
		inputSet.popFeatures();
	}

	// Find the selected features
	reported_.clear();

	for(unsigned int w = 0; w < weakLearners_.size(); ++w) {
		weakLearners_[w].report(reported_);
	}

	std::sort(reported_.begin(), reported_.end());

	reported_.resize(std::unique(reported_.begin(), reported_.end()) -
					 reported_.begin());

	// Trivial mapping
	std::vector<unsigned int> mapping(nbFeatures);

	for(unsigned int f = 0; f < reported_.size(); ++f) {
		mapping[reported_[f]] = f;
	}

	for(unsigned int w = 0; w < weakLearners_.size(); ++w) {
		weakLearners_[w].remap(mapping);
	}
}

void AdaBoost::distribution(InputSet& inputSet,
							unsigned int sample,
							scalar_t* distr) const {
	std::fill_n(distr, inputSet.nbLabels(), 0.0);

	inputSet.pushFeatures(reported_);

	for(unsigned int r = 0; r < weakLearners_.size(); ++r) {
		distr[weakLearners_[r].classify(inputSet, sample)] += alphas_[r];
	}

	inputSet.popFeatures();
}

void AdaBoost::print(std::ostream& out) const {
	out << "[AdaBoost::print] AdaBoost classifier (#weak learners: "
		<< weakLearners_.size() << ')' << std::endl;

	// Trivial mapping
	std::vector<unsigned int> mapping(*std::max_element(reported_.begin(),
														reported_.end()) + 1);

	for(unsigned int f = 0; f < reported_.size(); ++f) {
		mapping[reported_[f]] = f;
	}

	for(unsigned int w = 0; w < weakLearners_.size(); ++w) {
		out << "Alpha: " << alphas_[w] << ", ";
		const_cast<WeakTree&>(weakLearners_[w]).remap(reported_);
		weakLearners_[w].print(out);
		const_cast<WeakTree&>(weakLearners_[w]).remap(mapping);
	}
}

void AdaBoost::report(std::vector<unsigned int>& features) const {
	features.insert(features.end(), reported_.begin(), reported_.end());
}

AdaBoost::WeakTree::WeakTree(unsigned int maxDepth) : maxDepth_(maxDepth) {
	// Nothing to do
}

void AdaBoost::WeakTree::train(InputSet& inputSet,
							   const std::vector<std::vector<double> >& costs) {
	// Get the number of samples and labels
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Compute the sums of the columns of the cost matrix
	std::vector<double> sums(nbLabels);

	for(unsigned int s = 0; s < nbSamples; ++s) {
		for(unsigned int l = 0; l < nbLabels; ++l) {
			sums[l] += costs[s][l];
		}
	}

	// Vector of indices over the samples
	std::vector<unsigned int> indices(nbSamples);

	// Make the indices range from o to nbSamples - 1
	for(unsigned int s = 0; s < nbSamples; ++s) {
		indices[s] = s;
	}

	// Default depth of the tree
	unsigned int depth = maxDepth_;

	if(!maxDepth_) {
		maxDepth_ = std::ceil(std::log(double(nbLabels)) / std::log(2.0));
	}

	train(inputSet, costs, sums, &indices[0], nbSamples, maxDepth_);
	maxDepth_ = depth;
}

unsigned int AdaBoost::WeakTree::classify(InputSet& inputSet,
										  unsigned int sample) const {
	// Get the features of the sample to classifiy
	const scalar_t* features = inputSet.features(sample);

	assert(!nodes_.empty());

	// The node currently selected
	unsigned int current = 0;

	// If the node has children, find which one is selected
	while(!(current & 0x80000000U)) {
		assert(current < nodes_.size());
		current = nodes_[current].labels_[features[nodes_[current].feature_] >
										  nodes_[current].split_];
	}

	return current & 0x7FFFFFFFU;
}

void AdaBoost::WeakTree::print(std::ostream& out) const {
	out << "WeakTree classifier (#nodes: " << nodes_.size() << ", max depth: "
		<< maxDepth_ << ')';

	print(out, 0, 0);

	out << std::endl;
}

void AdaBoost::WeakTree::report(std::vector<unsigned int>& features) const {
	for(unsigned int n = 0; n < nodes_.size(); ++n) {
		features.push_back(nodes_[n].feature_);
	}
}

void AdaBoost::WeakTree::remap(const std::vector<unsigned int>& mapping) {
	for(unsigned int n = 0; n < nodes_.size(); ++n) {
		nodes_[n].feature_ = mapping[nodes_[n].feature_];
	}
}

bool AdaBoost::WeakTree::train(const InputSet& inputSet,
							   const std::vector<std::vector<double> >& costs,
							   const std::vector<double>& sums,
							   unsigned int* indices,
							   unsigned int nbSamples,
							   unsigned int depth) {
	// Their should be costs for all the training samples, not only the
	// selected subset
	assert(inputSet.nbSamples() == costs.size());

	// Get the number of features and labels
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();

	// The currently predicted label is the one with minimum cost
	unsigned int l = std::min_element(sums.begin(), sums.end()) - sums.begin();

	Node node;
	node.feature_ = 0;
	node.split_ = 0.0;
	node.labels_[0] = l | 0x80000000U;
	node.labels_[1] = l | 0x80000000U;

	// Cost of the node so far
	double bestCost = sums[l];

	// Sums of the columns of the cost matrix before the split
	std::vector<double> partialSums(nbLabels);

	// Sums of the columns of the cost matrix before the best split
	std::vector<double> bestSums;

	// Try to split on every feature
	for(unsigned int f = 0; f < nbFeatures; ++f) {
		// Set the partial sums to zero
		std::fill(partialSums.begin(), partialSums.end(), 0.0);

		// Get the samples (the values of the current feature)
		const scalar_t* samples = inputSet.samples(f);

		// Sort the indices according to the current feature
		Utils::sort(indices, samples, nbSamples);

		// Update the weight of the samples before the split by adding the
		// weight of each sample at every iteration. The weight of the samples
		// after the split is just the sum of the costs minus that weight
		for(unsigned int s = 0; s < nbSamples - 1; ++s) {
			const unsigned int index = indices[s];
			const unsigned int nextIndex = indices[s + 1];

			for(unsigned int l = 0; l < nbLabels; ++l) {
				partialSums[l] += costs[index][l];
			}

			// Try only to split in-between two samples with different feature
			if(Utils::less(samples[index], samples[nextIndex])) {
				double leftCost = partialSums[0];
				double rightCost = sums[0] - partialSums[0];
				unsigned int leftLabel = 0;
				unsigned int rightLabel = 0;

				for(unsigned int l = 1; l < nbLabels; ++l) {
					if(partialSums[l] < leftCost) {
						leftCost = partialSums[l];
						leftLabel = l;
					}
					else if(sums[l] - partialSums[l] < rightCost) {
						rightCost = sums[l] - partialSums[l];
						rightLabel = l;
					}
				}

				double cost = leftCost + rightCost;

				if(cost < bestCost) {
					node.feature_ = f;
					node.split_ = (samples[index] + samples[nextIndex]) / 2;
					node.labels_[0] = leftLabel  | 0x80000000U;
					node.labels_[1] = rightLabel | 0x80000000U;
					bestCost = cost;
					bestSums = partialSums;
				}
			}
		}
	}

	// If the node is constant then there is nothing to do
	if(bestSums.empty()) {
		// Add the node only if it is the only one
		if(depth == maxDepth_) {
			nodes_.push_back(node);
		}

		return false;
	}

	// Add the node
	const unsigned int index = nodes_.size();
	nodes_.push_back(node);

	// If the maximum depth was reached then there is nothing to do
	if(depth <= 1) {
		return true;
	}

	// Save the current maximum depth
	const unsigned int maxDepth = maxDepth_;

	// Build the indices for the left and the right children by partitioning
	// the indices in-place around the pivot
	unsigned int rightIndex = Utils::partition(indices,
											   inputSet.samples(node.feature_),
											   nbSamples,
											   node.split_);

	// Train the first child
	unsigned int child = nodes_.size();

	if(train(inputSet, costs, bestSums, indices, rightIndex, depth - 1)) {
		nodes_[index].labels_[0] = child;
	}

	// Train the second child
	child = nodes_.size();

	for(unsigned int l = 0; l < nbLabels; ++l) {
		bestSums[l] = sums[l] - bestSums[l];
	}

	if(train(inputSet, costs, bestSums, indices + rightIndex,
			 nbSamples - rightIndex, depth - 1)) {
		nodes_[index].labels_[1] = child;
	}

	return true;
}

void AdaBoost::WeakTree::print(std::ostream& out,
							   unsigned int index,
							   unsigned int depth) const {
	out << std::endl;

	// Output as many '|' as needed
	for(unsigned int d = 0; d < depth; ++d) {
		out << '|' << '\t';
	}

	out << "feature " << nodes_[index].feature_ << " <= "
		<< nodes_[index].split_ << ':';

	if(nodes_[index].labels_[0] & 0x80000000U) {
		out << ' ' << (nodes_[index].labels_[0] & 0x7FFFFFFFU);
	}
	else {
		print(out, nodes_[index].labels_[0], depth + 1);
	}

	out << std::endl;

	// Output as many '|' as needed
	for(unsigned int d = 0; d < depth; ++d) {
		out << '|' << '\t';
	}

	out << "feature " << nodes_[index].feature_ << "  > "
		<< nodes_[index].split_ << ':';

	if(nodes_[index].labels_[1] & 0x80000000U) {
		out << ' ' << (nodes_[index].labels_[1] & 0x7FFFFFFFU);
	}
	else {
		print(out, nodes_[index].labels_[1], depth + 1);
	}
}
