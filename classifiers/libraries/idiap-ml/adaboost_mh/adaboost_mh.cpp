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


/// \file	adaboost_mh.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Apr 4, 2011

#include "adaboost_mh.h"

#include <idiap-ml/dataset/dataset_subset.h>
#include <idiap-ml/dataset/dataset_cache.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace ml;

AdaBoostMH::AdaBoostMH(unsigned int nbRounds,
					   unsigned int nbFeaturesPerHeuristic,
					   unsigned int maxDepth) : nbRounds_(nbRounds),
nbFeaturesPerHeuristic_(nbFeaturesPerHeuristic), maxDepth_(maxDepth) {
	// The number of rounds must be strictly positive
	if (!nbRounds) {
		throw std::invalid_argument("the number of rounds must be strictly "
									"positive");
	}
}

IClassifier* AdaBoostMH::clone() const {
	return new AdaBoostMH(*this);
}

void AdaBoostMH::train(const IDataSet& dataset) {
	// Get the number of samples, features, heuristics, and labels
	const unsigned int nbSamples = dataset.nbSamples();
	const unsigned int nbFeatures = dataset.nbFeatures();
	const unsigned int nbHeuristics = dataset.nbHeuristics();
	const unsigned int nbLabels = dataset.nbLabels();

	// Standard requirements
	if (nbSamples < 2 || !nbFeatures || nbLabels < 2 || !nbHeuristics) {
		throw std::invalid_argument("there must be at least 2 training "
									"samples, 1 feature, 2 labels, and 1 "
									"heuristic");
	}

	// Separate the features by heuristic
	std::vector<std::vector<unsigned int> > heuristics(nbHeuristics);

	for (unsigned int f = 0; f < nbFeatures; ++f) {
		heuristics[dataset.heuristic(f)].push_back(f);
	}

	// Hypotheses matrix
	if (!hypotheses_.empty()) {
		if (hypotheses_.size() != nbSamples) {
			throw std::logic_error("the classifier was already trained with a "
								   "different number of samples");
		}
		else if (hypotheses_[0].size() != nbLabels) {
			throw std::logic_error("the classifier was already trained with a "
								   "different number of labels");
		}

		// Unmap the weak learners
		for (unsigned int w = 0; w < weakLearners_.size(); ++w) {
			weakLearners_[w].remap(reported_);
		}
	}
	else {
		loss_ = nbLabels;
		trainError_ = 1.0;
		hypotheses_.clear();
		hypotheses_.resize(nbSamples, std::vector<double>(nbLabels));
	}

	// Costs matrix
	std::vector<std::vector<double> > costs(nbSamples,
											std::vector<double>(nbLabels));

	// Prediction matrix (responses of the current weak learner)
	std::vector<std::vector<char> > predictions(nbSamples,
												std::vector<char>(nbLabels));

	// The total number of features to sample per round
	unsigned int nbFeaturesPerRound = 0;

	// The dataset cache
	std::auto_ptr<DataSetCache> cache;

	// The subset of features currently cached
	std::auto_ptr<FeatureSubSet> cached;

	// The subset of features currently selected
	std::auto_ptr<FeatureSubSet> subset;

	// The indices of the features currently selected
	std::vector<unsigned int> indices;

	// The number of rounds that can fit in the cache
	const unsigned int maximumCacheSize = (1U << 30U) / sizeof(float);
	unsigned int nbCachedRounds = 0;

	// Flag indicating that the whole dataset fit in the cache
	bool fit = false;

	if (nbFeaturesPerHeuristic_) {
		for (unsigned int h = 0; h < nbHeuristics; ++h) {
			nbFeaturesPerRound +=
				std::min(nbFeaturesPerHeuristic_,
					 	 static_cast<unsigned int>(heuristics[h].size()));
		}

		// Make sure that the cache is big enough to accommodate at least one
		// boosting round
		if (nbSamples * nbFeaturesPerRound > maximumCacheSize) {
			throw std::length_error("The cache is too small to accomodate even "
									"one boosting round");
		}

		// Is it better to cache some rounds or all the dataset?
		if (nbSamples * nbFeatures <= maximumCacheSize &&
			nbFeatures <= nbFeaturesPerRound * nbRounds_) {
			nbCachedRounds = 1;
			fit = true;
			cache.reset(new DataSetCache(&dataset, true));
		}
		else {
			nbCachedRounds =
				std::min(maximumCacheSize / (nbSamples * nbFeaturesPerRound),
						 nbRounds_);
		}

		std::cout << "[AdaBoostMH::train] #features/round: " << nbFeaturesPerRound
				  << "#rounds/cache: " << (fit ? nbRounds_ : nbCachedRounds) << std::endl;
	}
	else {
		if (nbSamples * nbFeatures > maximumCacheSize) {
			throw std::length_error("The cache is too small to accomodate even "
									"one boosting round");
		}

		cache.reset(new DataSetCache(&dataset, true));
	}

	std::cout << "   Round"
			  << "           Edge"
			  << "          Alpha"
			  << "           Loss"
			  << " Training error" << std::endl;

	// Do nbRounds rounds of boosting
	for (unsigned int r = 0; r < nbRounds_; ++r) {
		// If we need to sample features
		if (nbFeaturesPerHeuristic_) {
			unsigned int i = r % nbCachedRounds;

			if (i == 0) {
				unsigned int nbRounds = std::min(nbCachedRounds, nbRounds_ - r);
				indices.clear();

				for (unsigned int j = 0; j < nbRounds; ++j) {
					for (unsigned int h = 0; h < nbHeuristics; ++h) {
						if (heuristics[h].size() > nbFeaturesPerHeuristic_) {
							std::random_shuffle(heuristics[h].begin(),
												heuristics[h].end());
							std::sort(heuristics[h].begin(),
									  heuristics[h].begin() +
									  nbFeaturesPerHeuristic_);
							indices.insert(indices.end(),
										   heuristics[h].begin(),
										   heuristics[h].begin() +
										   nbFeaturesPerHeuristic_);
						}
						else {
							indices.insert(indices.end(),
										   heuristics[h].begin(),
										   heuristics[h].end());
						}
					}
				}

				if (!fit) {
					cached.reset(new FeatureSubSet(&dataset,
												   nbFeaturesPerRound * nbRounds,
												   &indices[0]));
					cache.reset(new DataSetCache(cached.get(), true));
				}
			}
			else if (indices.size() > nbFeaturesPerRound) {
				std::copy_backward(indices.begin() + nbFeaturesPerRound,
								   indices.end(),
								   indices.end() - nbFeaturesPerRound);
				indices.resize(indices.size() - nbFeaturesPerRound);
			}

			if (fit) {
				subset.reset(new FeatureSubSet(cache.get(), nbFeaturesPerRound,
											   &indices[0]));
			}
			else {
				std::vector<unsigned int> indexes(nbFeaturesPerRound);

				for (unsigned int j = 0; j < nbFeaturesPerRound; ++j) {
					indexes[j] = i * nbFeaturesPerRound + j;
				}

				subset.reset(new FeatureSubSet(cache.get(), nbFeaturesPerRound,
											   &indexes[0]));
			}
		}

		// Compute the current cost matrix from the hypothesis
		double sum = 0.0;

		for (unsigned int s = 0; s < nbSamples; ++s) {
			for (unsigned int l = 0; l < nbLabels; ++l) {
				double y = (l == dataset.label(s)) ? 1.0 : -1.0;
				costs[s][l] = std::exp(-y * hypotheses_[s][l]);
				sum += costs[s][l];
			}
		}

		// Normalize the weights of the samples
		for(unsigned int s = 0; s < nbSamples; ++s) {
			std::transform(costs[s].begin(), costs[s].end(), costs[s].begin(),
						   std::bind2nd(std::multiplies<double>(), 1.0 / sum));
		}

		// Train a new weak learner on the current cost matrix
		WeakTreeMH weakLearner(maxDepth_);

		// Save the predictions of the weak learner
		if (nbFeaturesPerHeuristic_) {
			weakLearner.train(*subset, costs);

			// Save the predictions of the weak learner
			for (unsigned int s = 0; s < nbSamples; ++s) {
				weakLearner.distribution(*subset, s, &predictions[s][0]);
			}

			weakLearner.remap(indices);
		}
		else {
			weakLearner.train(*cache, costs);

			// Save the predictions of the weak learner
			for (unsigned int s = 0; s < nbSamples; ++s) {
				weakLearner.distribution(*cache, s, &predictions[s][0]);
			}
		}

		// Compute its edge
		double edge = 0.0;

		for(unsigned int s = 0; s < nbSamples; ++s) {
			for(unsigned int l = 0; l < nbLabels; ++l) {
				edge += (l == dataset.label(s)) ? predictions[s][l] * costs[s][l] :
												 -predictions[s][l] * costs[s][l];
			}
		}

		// Find the alpha minimizing the loss
		if (edge < 1e-6) {
			std::cerr << "[AdaBoostMH::train] Could not find a weak learner"
					  << std::endl;
			break;
		}
		else if (edge > 1.0 - 1e-6) {
			std::cout << "[AdaBoostMH::train] (Nearly) Perfect weak learner "
						 "found" << std::endl;
			weakLearners_.push_back(weakLearner);
			alphas_.push_back(10.0);
			break;
		}

		// Update the loss
		loss_ *= std::sqrt(1.0 - edge * edge);

		// Compute the weight to give to the weak learner
		const double alpha = 0.5 * std::log((1.0 + edge) / (1.0 - edge));

		weakLearners_.push_back(weakLearner);
		alphas_.push_back(alpha);

		// Update the hypotheses and compute the training error
		trainError_ = 0.0;

		for (unsigned int s = 0; s < nbSamples; ++s) {
			for(unsigned int l = 0; l < nbLabels; ++l) {
				hypotheses_[s][l] += predictions[s][l] * alpha;
			}

			unsigned int predicted = static_cast<unsigned int>(
				std::max_element(hypotheses_[s].begin(), hypotheses_[s].end()) -
				hypotheses_[s].begin());

			if (predicted != dataset.label(s)) {
				trainError_ += 1.0 / nbSamples;
			}
		}

		std::cout << std::setw(8) << (r + 1)
				  << std::setw(15) << edge
				  << std::setw(15) << alpha
				  << std::setw(15) << loss_
				  << std::setw(15) << trainError_ << std::endl;
	}

	// Find the selected features
	for (unsigned int w = 0; w < weakLearners_.size(); ++w) {
		weakLearners_[w].report(reported_);
	}

	std::sort(reported_.begin(), reported_.end());

	reported_.resize(std::unique(reported_.begin(), reported_.end()) -
					 reported_.begin());

	// Trivial mapping
	std::vector<unsigned int> mapping(nbFeatures);

	for (unsigned int f = 0; f < reported_.size(); ++f) {
		mapping[reported_[f]] = f;
	}

	for (unsigned int w = 0; w < weakLearners_.size(); ++w) {
		weakLearners_[w].remap(mapping);
	}
}

void AdaBoostMH::distribution(const IDataSet& dataset,
							  unsigned int sample,
							  double* distr) const {
	const unsigned int nbLabels = dataset.nbLabels();

	std::fill_n(distr, nbLabels, 0.0);

	// Cache the sample to classify
	const unsigned int nbFeatures = static_cast<unsigned int>(reported_.size());
	SampleSubSet sampleSubSet(&dataset, 1, &sample);
	FeatureSubSet featureSubSet(&sampleSubSet, nbFeatures, &reported_[0]);
	DataSetCache cache(&featureSubSet);

	std::vector<char> tmp(nbLabels);

	for (unsigned int w = 0; w < weakLearners_.size(); ++w) {
		weakLearners_[w].distribution(cache, 0, &tmp[0]);

		for(unsigned int l = 0; l < nbLabels; ++l) {
			distr[l] += alphas_[w] * tmp[l];
		}
	}
}

void AdaBoostMH::report(std::vector<unsigned int>& features) const {
	features.insert(features.end(), reported_.begin(), reported_.end());
}

void AdaBoostMH::load(const json::Value& value) {
	if (value["name"].string() != "AdaBoostMH") {
		throw std::invalid_argument("the input classifier is not AdaBoostMH");
	}

	loss_ = value["loss"];
	trainError_ = value["trainError"];
	weakLearners_.resize(value["weakLearners"].array().size());
	alphas_.assign(value["alphas"].array().begin(), value["alphas"].array().end());
	reported_.assign(value["reported"].array().begin(), value["reported"].array().end());
	hypotheses_.resize(value["hypotheses"].array().size());

	for (unsigned int w = 0; w < weakLearners_.size(); ++w) {
		weakLearners_[w].load(value["weakLearners"][w]);
	}

	for (unsigned int s = 0; s < hypotheses_.size(); ++s) {
		hypotheses_[s].assign(value["hypotheses"][s].array().begin(),
							  value["hypotheses"][s].array().end());
	}
}

void AdaBoostMH::save(json::Value& value) const {
	value["name"] = "AdaBoostMH";
	value["loss"] = loss_;
	value["trainError"] = trainError_;
	value["weakLearners"].array().resize(weakLearners_.size());
	value["alphas"].array().assign(alphas_.begin(), alphas_.end());
	value["reported"].array().assign(reported_.begin(), reported_.end());
	value["hypotheses"].array().resize(hypotheses_.size());

	for (unsigned int w = 0; w < weakLearners_.size(); ++w) {
		json::Value weakLearner;
		weakLearners_[w].save(weakLearner);
		value["weakLearners"][w] = weakLearner;
	}

	for (unsigned int s = 0; s < hypotheses_.size(); ++s) {
		value["hypotheses"][s].array().assign(hypotheses_[s].begin(),
											  hypotheses_[s].end());
	}
}

void AdaBoostMH::print(std::ostream& out) const {
	out << "[AdaBoostMH::print] AdaBoost.MH classifier (#weak learners: "
		<< weakLearners_.size() << ')' << std::endl;

	// Trivial mapping
	std::vector<unsigned int> mapping(*std::max_element(reported_.begin(),
														reported_.end()) + 1);

	for (unsigned int f = 0; f < reported_.size(); ++f) {
		mapping[reported_[f]] = f;
	}

	for (unsigned int w = 0; w < weakLearners_.size(); ++w) {
		out << "Alpha: " << alphas_[w] << ", ";
		const_cast<WeakTreeMH&>(weakLearners_[w]).remap(reported_);
		weakLearners_[w].print(out);
		const_cast<WeakTreeMH&>(weakLearners_[w]).remap(mapping);
	}
}

double AdaBoostMH::trainError() const {
	return trainError_;
}
