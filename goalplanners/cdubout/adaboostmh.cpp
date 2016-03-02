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


// Author: Charles Dubout (charles.dubout@idiap.ch)
// Parameters:
//   NB_ROUNDS: number of Boosting rounds (default: 100)

#include <mash-goalplanning/planner.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

using namespace Mash;
using namespace std;

//------------------------------------------------------------------------------
// The 'AdaBoostMH' goal-planner class
//------------------------------------------------------------------------------
class AdaBoostMH: public Planner
{
	//_____ Construction / Destruction __________
public:
	AdaBoostMH();
	virtual ~AdaBoostMH();
	
	//_____ Methods to implement __________
public:
	virtual bool setup(const tExperimentParametersList & parameters);
	virtual bool loadModel(PredictorModel & model);
	virtual bool learn(ITask * task);
	virtual unsigned int chooseAction(IPerception * perception);
	virtual bool reportFeaturesUsed(tFeatureList & list);
	virtual bool saveModel(PredictorModel & model);
	
	//_____ Attributes __________
protected:
	// A multiclass stump suitable for AdaBoost.MH
	struct Stump
	{
		unsigned int feature_;
		scalar_t threshold_;
		vector<bool> signs_;
	};
	
	// Train a stump
	double trainStump(const vector<vector<scalar_t> > & features,
					  const vector<unsigned int> & labels,
					  const vector<vector<unsigned int> > & indices,
					  const vector<vector<double> > & weights,
					  const vector<double> & edges,
					  Stump & stump) const;
	
	// Parameters
	unsigned int nbRounds_;
	
	// The AdaBoost.MH classifier
	vector<scalar_t> alphas_;
	vector<Stump> stumps_;
	
	// Cache
	unsigned int nbFeaturesTotal_;
	unsigned int nbHeuristics_;
	unsigned int nbLabels_;
	vector<unsigned int> nbFeatures_;
	coordinates_t coordinates_;
};

//------------------------------------------------------------------------------
// Creation function of the goal-planner
//------------------------------------------------------------------------------
extern "C" Planner* new_planner()
{
	return new AdaBoostMH();
}

/************************* CONSTRUCTION / DESTRUCTION *************************/

AdaBoostMH::AdaBoostMH() : nbRounds_(100), nbFeaturesTotal_(0), nbHeuristics_(0), nbLabels_(0)
{
}


AdaBoostMH::~AdaBoostMH()
{
}

/************************* IMPLEMENTATION OF Classifier ***********************/

bool AdaBoostMH::setup(const tExperimentParametersList & parameters)
{
	tExperimentParametersIterator nbRounds = parameters.find("NB_ROUNDS");
	
	if ((nbRounds != parameters.end()) && (nbRounds->second.size() == 1))
		nbRounds_ = nbRounds->second.getInt(0);
	
	return true;
}

bool AdaBoostMH::loadModel(PredictorModel & model)
{
	return true;
}

namespace detail
{
	struct Compare
	{
		Compare(const scalar_t * features) : features_(features)
		{
		}
		
		bool operator()(unsigned int a, unsigned int b) const
		{
			return features_[a] < features_[b];
		}
		
		const scalar_t * features_;
	};
}

bool AdaBoostMH::learn(ITask * task)
{
	// Note: we only consider the first view
	
	// ROI stuff
	const dim_t size = task->perception()->viewSize(0);
	
	coordinates_.x = size.width / 2;
	coordinates_.y = size.height / 2;
	
	// Features stuff
	nbHeuristics_ = task->perception()->nbHeuristics();
	
	nbFeatures_.resize(nbHeuristics_);
	
	for (unsigned int h = 0; h < nbHeuristics_; ++h) {
		nbFeatures_[h] = task->perception()->nbFeatures(h);
		
		if (nbFeatures_[h] > 10) {
			outStream << "Error:  heuristic " << h << " has " << nbFeatures_[h] << " features."
					  << endl;
			return false;
		}
	}
	
	nbFeaturesTotal_ = accumulate(nbFeatures_.begin(), nbFeatures_.end(), 0U);
	
	vector<vector<scalar_t> > features(nbFeaturesTotal_); // features x samples
	vector<unsigned int> actions;
	
	// Cache everything
	vector<scalar_t> tmp(nbFeaturesTotal_);
	
	scalar_t minFeature = numeric_limits<scalar_t>::infinity();
	scalar_t maxFeature =-numeric_limits<scalar_t>::infinity();
	
	for (unsigned int t = 0; t < task->nbTrajectories(); ++t) {
		unsigned int indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		
		const unsigned int length = task->trajectoryLength(t);
		
		for (unsigned int i = 0; i < length; ++i) {
			for (unsigned int h = 0, offset = 0; h < nbHeuristics_; ++h) {
				if (!task->perception()->computeSomeFeatures(0, coordinates_, h, nbFeatures_[h],
															 indices, &tmp[offset])) {
					outStream << "Error: computeSomeFeatures(0, [" << coordinates_.x << ", "
							  << coordinates_.y << "], " << h << ", " << nbFeatures_[h]
							  << ", ...) returned false." << endl;
					return false;
				}
				
				offset += nbFeatures_[h];
			}
			
			for (unsigned int j = 0; j < nbFeaturesTotal_; ++j) {
				features[j].push_back(tmp[j]);
				minFeature = min(minFeature, tmp[j]);
				maxFeature = max(maxFeature, tmp[j]);
			}
			
			actions.push_back(task->suggestedAction());
			
			scalar_t reward;
			task->performAction(actions.back(), &reward);
		}
		
		task->reset();
	}
	
	// Boosting goes here
	const unsigned int nbSamples = actions.size();
	
	nbLabels_ = *max_element(actions.begin(), actions.end()) + 1;
	
	if (!nbSamples || !nbFeaturesTotal_ || !nbLabels_) {
		outStream << "Error: no samples, features or labels." << endl;
		return false;
	}
	
	outStream << "# features: " << nbFeaturesTotal_ << ", # samples: " << nbSamples
			  << ", # labels: " << nbLabels_ << ", min/max: " << minFeature << " / "
			  << maxFeature << endl;
	
	// Ordering of the samples on every sampled feature
	vector<vector<unsigned int> > indices(nbFeaturesTotal_);
	
	for (unsigned int f = 0; f < nbFeaturesTotal_; ++f) {
		indices[f].resize(nbSamples);
		
		for(unsigned int s = 0; s < nbSamples; ++s)
			indices[f][s] = s;
		
		sort(indices[f].begin(), indices[f].end(), detail::Compare(&features[f][0]));
	}
	
	// Set the distribution of weights uniformly
	vector<vector<double> > weights(nbLabels_);
	vector<vector<double> > hypotheses(nbLabels_);
	
	for (unsigned int l = 0; l < nbLabels_; ++l) {
		weights[l].resize(nbSamples);
		hypotheses[l].resize(nbSamples);
	}
	
	// AdaBoost.MH (average) loss
	double logLoss = 0.0;
	
	// Do nbRounds rounds of boosting
	for (unsigned int r = 0; r < nbRounds_; ++r) {
		// Compute the current weights matrix from the hypotheses
		double sumWeights = 0.0;
		
		for (unsigned int s = 0; s < nbSamples; ++s) {
			for (unsigned int l = 0; l < nbLabels_; ++l) {
				weights[l][s] = std::exp((l == actions[s]) ? -hypotheses[l][s] : hypotheses[l][s]);
				sumWeights += weights[l][s];
			}
		}
		
		// Compute the edges for every label
		vector<double> edges(nbLabels_, 0.0);
		
		for (unsigned int l = 0; l < nbLabels_; ++l)
			for (unsigned int s = 0; s < nbSamples; ++s)
				edges[l] += (actions[s] == l) ? weights[l][s] : -weights[l][s];
		
		// Find the stump with the maximum edge
		Stump stump;
		const double edge = trainStump(features, actions, indices, weights, edges, stump) / sumWeights;
		
		// Update the loss
		logLoss += 0.5 * log10(1.0 - edge * edge);
		
		// Compute the weight to give to the weak learner and update the weights of the samples
		const double alpha = 0.5 * log((1.0 + edge) / (1.0 - edge));
		unsigned nbErrors = 0;
		
		for (unsigned int s = 0; s < nbSamples; ++s) {
			// The label with the maximum hypothesis
			double maxScore = -numeric_limits<double>::infinity();
			unsigned int label;
			
			bool phi = features[stump.feature_][s] >= stump.threshold_;
			
			for (unsigned int l = 0; l < nbLabels_; ++l) {
				hypotheses[l][s] += (phi ^ stump.signs_[l]) ? -alpha : alpha;
				
				if (hypotheses[l][s] > maxScore) {
					maxScore = hypotheses[l][s];
					label = l;
				}
			}
			
			if(label != actions[s])
				++nbErrors;
		}
		
		outStream << "AdaBoost.MH, round: " << r << ", log10(loss): " << logLoss << ", edge: "
				  << edge << ", training error: " << (static_cast<double>(nbErrors) / nbSamples)
				  << '.' << endl;
		
		// Add the stumps
		alphas_.push_back(alpha);
		stumps_.push_back(stump);
	}
	
	return true;
}

unsigned int AdaBoostMH::chooseAction(IPerception * perception)
{
	if (!nbFeaturesTotal_ || alphas_.empty() || (nbLabels_ < 2))
		return 0;
	
	vector<scalar_t> features(nbFeaturesTotal_);
	
	for (unsigned int h = 0, offset = 0; h < nbHeuristics_; ++h) {
		unsigned int indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		
		if (!perception->computeSomeFeatures(0, coordinates_, h, nbFeatures_[h], indices,
											 &features[offset])) {
			outStream << "Error: computeSomeFeatures(0, [" << coordinates_.x << ", "
					  << coordinates_.y << "], " << h << ", " << nbFeatures_[h]
					  << ", ...) returned false." << endl;
			return false;
		}
		
		offset += nbFeatures_[h];
	}
	
	vector<double> distribution(nbLabels_);
	
	for (unsigned int w = 0; w < alphas_.size(); ++w) {
		const bool phi = features[stumps_[w].feature_] >= stumps_[w].threshold_;
		
		for(unsigned int l = 0; l < nbLabels_; ++l)
			distribution[l] += (phi ^ stumps_[w].signs_[l]) ? -alphas_[w] : alphas_[w];
	}
	
	return max_element(distribution.begin(), distribution.end()) - distribution.begin();
}

bool AdaBoostMH::reportFeaturesUsed(tFeatureList & list)
{
	tFeatureList offsets;
	
	for (unsigned int h = 0; h < nbHeuristics_; ++h)
		for (unsigned int f = 0; f < nbFeatures_[h]; ++f)
			offsets.push_back(tFeature(h, f));
	
	for (unsigned int w = 0; w < stumps_.size(); ++w)
		list.push_back(offsets[stumps_[w].feature_]);
	
	return true;
}

bool AdaBoostMH::saveModel(PredictorModel & model)
{
	return true;
}

double AdaBoostMH::trainStump(const vector<vector<scalar_t> > & features,
							  const vector<unsigned int> & labels,
							  const vector<vector<unsigned int> > & indices,
							  const vector<vector<double> > & weights,
							  const vector<double> & edges,
							  Stump & stump) const
{
	if (features.empty() || labels.empty() || (features.size() != indices.size()) ||
		weights.empty() || (weights.size() != edges.size()))
		return 0.0;
	
	const unsigned int nbFeatures = features.size();
	const unsigned int nbSamples = labels.size();
	const unsigned int nbLabels = weights.size();
	
	// Initialize the stump
	stump.feature_ = 0;
	stump.threshold_ = -numeric_limits<scalar_t>::infinity();
	stump.signs_.resize(nbLabels);
	fill(stump.signs_.begin(), stump.signs_.end(), false);
	
	// The best sum of absolute values of edges so far
	double sumEdges = 0.0;
	
	for (unsigned int l = 0; l < nbLabels; ++l) {
		stump.signs_[l] = edges[l] >= 0.0;
		sumEdges += abs(edges[l]);
	}
	
	for (unsigned int f = 0; f < nbFeatures; ++f) {
		// The right edges are simply edges - leftEdges
		vector<double> leftEdges(nbLabels, 0.0);
		
		// Try to split in between every sample
		for (unsigned int s = 0; s < nbSamples - 1; ++s) {
			const unsigned int index = indices[f][s];
			const unsigned int nextIndex = indices[f][s + 1];
			const scalar_t feature = features[f][index];
			const scalar_t nextFeature = features[f][nextIndex];
			const unsigned int label = labels[index];
			
			// Include the current sample in the left edge
			double sum = 0.0;
			
			for (unsigned int l = 0; l < nbLabels; ++l) {
				leftEdges[l] += (label == l) ? weights[l][index] : -weights[l][index];
				sum += abs(edges[l] - 2.0 * leftEdges[l]);
			}
			
			// If a stump can be put in between and with a better sum of edges
			if((feature < nextFeature) && (sum > sumEdges)) {
				stump.feature_ = f;
				stump.threshold_ = (feature + nextFeature) / 2;
				
				for (unsigned int l = 0; l < nbLabels; ++l)
					stump.signs_[l] = edges[l] >= 2.0 * leftEdges[l];
				
				sumEdges = sum;
			}
		}
	}
	
	return sumEdges;
}
