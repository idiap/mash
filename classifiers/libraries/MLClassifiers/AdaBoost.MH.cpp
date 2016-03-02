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
/// @file MLClassifiers/AdaBoost.MH.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.02.09
/// @version 2.2
//------------------------------------------------------------------------------

#include "AdaBoost.MH.h"
#include "Utils.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace ML;

AdaBoostMH::AdaBoostMH(unsigned int nbRounds,
					   unsigned int nbFeaturesPerHeuristic,
					   unsigned int maxDepth,
					   double tca)
: nbRounds_(nbRounds), nbFeaturesPerHeuristic_(nbFeaturesPerHeuristic),
  maxDepth_(maxDepth), tca_(tca) {
	// The number of rounds must be strictly positive
	assert(nbRounds);

	// As well as the maximum depth
	assert(maxDepth);

	// The TCA threshold must be positive
	assert(tca >= 0.0);
}

Classifier* AdaBoostMH::clone() const {
	return new AdaBoostMH(*this);
}

void AdaBoostMH::train(InputSet& inputSet) {
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

	std::cout << "[AdaBoostMH::train] #samples: " << nbSamples << ", #features: "
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

	// Weights matrix
	const double w = 1.0 / (nbSamples * nbLabels);
	std::vector<std::vector<double> > weights(nbSamples,
											  std::vector<double>(nbLabels, w));

	// Hypotheses matrix
	std::vector<std::vector<double> > hypotheses(nbSamples,
											  	 std::vector<double>(nbLabels));

	// Prediction matrix (responses of the current weak learner)
	std::vector<std::vector<char> > predictions(nbSamples,
												std::vector<char>(nbLabels));

	// Previous predictions
	std::vector<std::vector<std::vector<char> > > oldPredictions;

	// Flag indicating that there is features in the cache
	bool cached = false;

	// If we need to sample features
	std::vector<unsigned int> indices;

	// AdaBoost.MH (average) loss
	double loss = nbLabels;

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

		// Train a new weak learner on the current weights matrix
		WeakTree weakLearner(maxDepth_);
		weakLearner.train(inputSet, weights);

		// Save the predictions of the weak learner and compute its edge
		double edge = 0.0;

		for(unsigned int s = 0; s < nbSamples; ++s) {
			weakLearner.distribution(inputSet, s, &predictions[s][0]);

			for(unsigned int l = 0; l < nbLabels; ++l) {
				edge += (l == labels[s]) ? predictions[s][l] * weights[s][l] :
										  -predictions[s][l] * weights[s][l];
			}
		}

		// If we sampled features
		if(nbFeaturesPerHeuristic_) {
			weakLearner.remap(indices);

			if(nbCachedRounds > 1) {
				inputSet.popFeatures();
			}
		}

		if(edge < 1e-6) {
			std::cerr << "[AdaBoostMH::train] Couldn't find a weak learner."
					  << std::endl;
			break;
		}
		else if(Utils::geq(edge, 1.0)) {
			std::cout << "[AdaBoostMH::train] (Nearly) Perfect weak learner found!"
					  << std::endl;
			weakLearners_.push_back(weakLearner);
			alphas_.push_back(1.0);
			break;
		}

		// Update the loss
		loss *= std::sqrt(1.0 - edge * edge);

		// Compute the weight to give to the weak learner
		double alpha = 0.5 * std::log((1.0 + edge) / (1.0 - edge));

		weakLearners_.push_back(weakLearner);
		alphas_.push_back(alpha);

		if(tca_) {
			oldPredictions.push_back(predictions);
		}

		// Update the hypotheses and compute the training error
		double sum = 0.0;
		double trainingError = 0;

		for(unsigned int s = 0; s < nbSamples; ++s) {
			for(unsigned int l = 0; l < nbLabels; ++l) {
				double y = (l == labels[s]) ? 1.0 : -1.0;
				hypotheses[s][l] += predictions[s][l] * alpha;
				weights[s][l] = std::exp(-y * hypotheses[s][l]);
				sum += weights[s][l];
			}

			unsigned int predicted = std::max_element(hypotheses[s].begin(),
													  hypotheses[s].end()) -
									 hypotheses[s].begin();

			if(predicted != labels[s]) {
				trainingError += 1.0 / nbSamples;
			}
		}

		// Normalize the weights of the samples
		for(unsigned int s = 0; s < nbSamples; ++s) {
			std::transform(weights[s].begin(), weights[s].end(),
						   weights[s].begin(),
						   std::bind2nd(std::multiplies<double>(), 1.0 / sum));
		}

		std::cout.precision(4);

		std::cout << "[AdaBoostMH::train] Round: " << (r + 1) << '/'
				  << nbRounds_ << ", edge: " << edge << ", alpha: " << alpha
				  << ", loss: " << loss << ", training error: " << trainingError
				  << '.' << std::endl;

		// Totally Corrected Step (TCA)
		for(unsigned int w = 0, i = 0, z = 10 * (r + 1); tca_ && i <= r && z;
			++w, ++i, w %= r + 1, --z) {
			if(alphas_[w] > tca_) {
				// Compute the edge of the weak learner under the new weights
				double edge = 0.0;

				for(unsigned int s = 0; s < nbSamples; ++s) {
					for(unsigned int l = 0; l < nbLabels; ++l) {
						edge += (l == labels[s]) ? oldPredictions[w][s][l] * weights[s][l] :
												  -oldPredictions[w][s][l] * weights[s][l];
					}
				}

				// The edge need to be positive
				double sign = (edge > 0.0) ? 1.0 : -1.0;
				edge = std::abs(edge);
				alpha = sign * 0.5 * std::log((1.0 + edge) / (1.0 - edge));

				// If the edge is non-zero, update the weight of the weak learner
				// and the weights of the samples
				if(alpha >= tca_) {
					// Update the loss
					loss *= std::sqrt(1.0 - edge * edge);

					// Compute the new alpha
					alpha = sign * 0.5 * std::log((1.0 + edge) / (1.0 - edge));
					sum = 0.0;

					// Update the weight of the weak learner
					alphas_[w] += alpha;

					// Update the weights of the samples
					for(unsigned int s = 0; s < nbSamples; ++s) {
						for(unsigned int l = 0; l < nbLabels; ++l) {
							double y = (l == labels[s]) ? 1.0 : -1.0;
							hypotheses[s][l] += oldPredictions[w][s][l] * alpha;
							weights[s][l] = std::exp(-y * hypotheses[s][l]);
							sum += weights[s][l];
						}
					}

					// Normalize the weights of the samples
					for(unsigned int s = 0; s < nbSamples; ++s) {
						std::transform(weights[s].begin(), weights[s].end(),
									   weights[s].begin(),
									   std::bind2nd(std::multiplies<double>(), 1.0 / sum));
					}

					std::cout << "[AdaBoostMH::train] TCA, alpha[" << w << "] += "
							  << alpha << " (" << alphas_[w] << "), loss: "
							  << loss << '.' << std::endl;

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

void AdaBoostMH::distribution(InputSet& inputSet,
							  unsigned int sample,
							  scalar_t* distr) const {
	const unsigned int nbLabels = inputSet.nbLabels();

	std::fill_n(distr, nbLabels, 0.0);

	inputSet.pushFeatures(reported_);

	std::vector<char> tmp(nbLabels);

	for(unsigned int r = 0; r < weakLearners_.size(); ++r) {
		weakLearners_[r].distribution(inputSet, sample, &tmp[0]);

		for(unsigned int l = 0; l < nbLabels; ++l) {
			distr[l] += alphas_[r] * tmp[l];
		}
	}

	inputSet.popFeatures();
}

void AdaBoostMH::print(std::ostream& out) const {
	out << "[AdaBoostMH::print] AdaBoost.MH classifier (#weak learners: "
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

void AdaBoostMH::report(std::vector<unsigned int>& features) const {
	features.insert(features.end(), reported_.begin(), reported_.end());
}

AdaBoostMH::WeakTree::WeakTree(unsigned int maxDepth) : maxDepth_(maxDepth) {
	// Nothing to do
}

void AdaBoostMH::WeakTree::train(InputSet& inputSet,
								 const std::vector<std::vector<double> >& weights) {
	// Get the number of samples and labels, as well as the labels themselves
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbLabels = inputSet.nbLabels();
	const unsigned int* labels = inputSet.labels();

	// Compute the edges of the labels
	std::vector<double> edges(nbLabels);

	for(unsigned int l = 0; l < nbLabels; ++l) {
		for(unsigned int s = 0; s < nbSamples; ++s) {
			edges[l] += (labels[s] == l) ? weights[s][l] : -weights[s][l];
		}
	}

	// Vector of indices over the samples
	std::vector<unsigned int> indices(nbSamples);

	// Make the indices range from o to nbSamples - 1
	for(unsigned int s = 0; s < nbSamples; ++s) {
		indices[s] = s;
	}

	train(inputSet, weights, edges, &indices[0], nbSamples, maxDepth_);
}

void AdaBoostMH::WeakTree::distribution(InputSet& inputSet,
										unsigned int sample,
										char* distr) const {
	// Get the features of the sample to classifiy
	const scalar_t* features = inputSet.features(sample);

	// The node currently selected
	unsigned int current = 0;

	// If the node has children, find which one is selected
	while(unsigned int n = nodes_[current].children_[features[nodes_[current].feature_] >
													 nodes_[current].split_]) {
		assert(n > current);
		current = n;
	}

	bool phi = features[nodes_[current].feature_] > nodes_[current].split_;

	for(unsigned int l = 0; l < nodes_[current].signs_.size(); ++l) {
		distr[l] = (phi == nodes_[current].signs_[l]) ? 1 : -1;
	}
}

void AdaBoostMH::WeakTree::print(std::ostream& out) const {
	out << "WeakTree classifier (#nodes: " << nodes_.size() << ", max depth: "
		<< maxDepth_ << ')';

	print(out, 0, 0);

	out << std::endl;
}

void AdaBoostMH::WeakTree::report(std::vector<unsigned int>& features) const {
	for(unsigned int n = 0; n < nodes_.size(); ++n) {
		features.push_back(nodes_[n].feature_);
	}
}

void AdaBoostMH::WeakTree::remap(const std::vector<unsigned int>& mapping) {
	for(unsigned int n = 0; n < nodes_.size(); ++n) {
		nodes_[n].feature_ = mapping[nodes_[n].feature_];
	}
}

bool AdaBoostMH::WeakTree::train(const InputSet& inputSet,
								 const std::vector<std::vector<double> >& weights,
								 const std::vector<double>& edges,
								 unsigned int* indices,
								 unsigned int nbSamples,
								 unsigned int depth) {
	// Their should be costs for all the training samples, not only the
	// selected subset
	assert(inputSet.nbSamples() == weights.size());

	// Get the number of features and labels, as well as the labels themselves
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();
	const unsigned int* labels = inputSet.labels();

	// Predict the best signs
	Node node;
	node.feature_ = 0;
	node.split_ = -std::numeric_limits<scalar_t>::infinity();
	node.signs_.resize(nbLabels);
	node.children_[0] = 0;
	node.children_[1] = 0;

	// The best sum of absolute values of edges so far
	double sumEdges = 0.0;

	for(unsigned int l = 0; l < nbLabels; ++l) {
		node.signs_[l] = (edges[l] >= 0.0);
		sumEdges += std::abs(edges[l]);
	}

	// Edges before the split
	std::vector<double> partialEdges(nbLabels);

	// Edges before the best split
	std::vector<double> bestEdges;

	// Try to split on every feature
	for(unsigned int f = 0; f < nbFeatures; ++f) {
		// Set the partial edges to zero
		std::fill(partialEdges.begin(), partialEdges.end(), 0.0);

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

			// Include the current sample in the partial edges
			double sum = 0.0;

			for(unsigned int l = 0; l < nbLabels; ++l) {
				partialEdges[l] += (labels[index] == l) ? weights[index][l] :
														 -weights[index][l];

				sum += std::abs(edges[l] - 2.0 * partialEdges[l]);
			}

			// Try only to split in-between two samples with different feature
			if(Utils::less(samples[index], samples[nextIndex]) && (sum > sumEdges)) {
				node.feature_ = f;
				node.split_ = (samples[index] + samples[nextIndex]) / 2;

				for(unsigned int l = 0; l < nbLabels; ++l) {
					node.signs_[l] = edges[l] >= 2.0 * partialEdges[l];
				}

				sumEdges = sum;
				bestEdges = partialEdges;
			}
		}
	}

	// If the node is constant then there is nothing to do
	if(bestEdges.empty()) {
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

	if(train(inputSet, weights, bestEdges, indices, rightIndex, depth - 1)) {
		nodes_[index].children_[0] = child;
	}

	// Train the second child
	child = nodes_.size();

	for(unsigned int l = 0; l < nbLabels; ++l) {
		bestEdges[l] = edges[l] - bestEdges[l];
	}

	if(train(inputSet, weights, bestEdges, indices + rightIndex,
			 nbSamples - rightIndex, depth - 1)) {
		nodes_[index].children_[1] = child;
	}

	return true;
}

void AdaBoostMH::WeakTree::print(std::ostream& out,
								 unsigned int index,
								 unsigned int depth) const {
	out << std::endl;

	// Output as many '|' as needed
	for(unsigned int d = 0; d < depth; ++d) {
		out << '|' << '\t';
	}

	out << "feature " << nodes_[index].feature_ << " <= "
		<< nodes_[index].split_ << ':';

	if(!nodes_[index].children_[0]) {
		out << ' ';

		for(unsigned int l = 0; l < nodes_[index].signs_.size(); ++l) {
			out << (nodes_[index].signs_[l] ? '-' : '+');
		}
	}
	else {
		print(out, nodes_[index].children_[0], depth + 1);
	}

	out << std::endl;

	// Output as many '|' as needed
	for(unsigned int d = 0; d < depth; ++d) {
		out << '|' << '\t';
	}

	out << "feature " << nodes_[index].feature_ << "  > "
		<< nodes_[index].split_ << ':';

	if(!nodes_[index].children_[1]) {
		out << ' ';

		for(unsigned int l = 0; l < nodes_[index].signs_.size(); ++l) {
			out << (nodes_[index].signs_[l] ? '+' : '-');
		}
	}
	else {
		print(out, nodes_[index].children_[1], depth + 1);
	}
}
