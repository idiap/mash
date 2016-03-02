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
/// @file MLInputSet/InputSet.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.04.08
/// @version 2.1
//------------------------------------------------------------------------------

#include "InputSet.h"

#include <algorithm>
#include <cassert>

using namespace ML;

InputSet::InputSet()
: sampleStack_(), featureStack_(), matrixCache_(sampleStack_, featureStack_),
  labelCache_(sampleStack_), weightCache_(sampleStack_),
  heuristicCache_(featureStack_), imageCache_(sampleStack_),
  coordinatesCache_(sampleStack_) {
	// Nothing to do
}

InputSet::~InputSet() {
	// Nothing to do
}

unsigned int InputSet::nbSamples() const {
	assert(!sampleStack_.empty());
	return sampleStack_.back().size();
}

unsigned int InputSet::nbFeatures() const {
	assert(!featureStack_.empty());
	return featureStack_.back().size();
}

unsigned int InputSet::nbLabels() const {
	assert(!nbLabelStack_.empty());
	return nbLabelStack_.back();
}

unsigned int InputSet::nbImages() const {
	return nbImages_;
}

unsigned int InputSet::nbHeuristics() const {
	return nbHeuristics_;
}

void InputSet::pushSamples(const std::vector<unsigned int>& indices) {
	// There must be indices
	assert(!indices.empty());

	// Add the vector on the sample stack
	sampleStack_.push_back(indices);

	// The current number of labels is equal to the previous one
	nbLabelStack_.push_back(nbLabelStack_.back());
}

void InputSet::popSamples() {
	// There must be a pushed stack
	assert(sampleStack_.size() > 1);

	sampleStack_.pop_back();
	nbLabelStack_.pop_back();

	matrixCache_.empty();
	labelCache_.empty();
	weightCache_.empty();
	imageCache_.empty();
	coordinatesCache_.empty();
}

void InputSet::pushFeatures(const std::vector<unsigned int>& indices) {
	// There must be indices
	assert(!indices.empty());

	// Add the vector on the feature stack
	featureStack_.push_back(indices);
}

void InputSet::popFeatures() {
	// There must be a pushed stack
	assert(featureStack_.size() > 1);

	featureStack_.pop_back();

	matrixCache_.empty();
	heuristicCache_.empty();
}

const scalar_t* InputSet::features(unsigned int sample) const {
	// If there is no cache
	if(matrixCache_.empty()) {
		std::vector<scalar_t> data;
		matrixCache(data);
		matrixCache_.swap(data);
	}

	return matrixCache_.data() + sample * nbFeatures();
}

const scalar_t* InputSet::samples(unsigned int feature) const {
	// If there is no cache
	if(matrixCache_.empty()) {
		std::vector<scalar_t> data;
		matrixCache(data);
		matrixCache_.swap(data);
	}

	return matrixCache_.dataT() + feature * nbSamples();
}

const unsigned int* InputSet::labels() const {
	// If there is no cache
	if(labelCache_.empty()) {
		std::vector<unsigned int> data;
		labelCache(data);
		labelCache_.swap(data);
	}

	return labelCache_.data();
}

const scalar_t* InputSet::weights() const {
	// If there is no cache
	if(weightCache_.empty()) {
		std::vector<scalar_t> data;
		weightCache(data);
		weightCache_.swap(data);
	}

	return weightCache_.data();
}

const unsigned int* InputSet::images() const {
	// If there is no cache
	if(imageCache_.empty()) {
		std::vector<unsigned int> data;
		imageCache(data);
		imageCache_.swap(data);
	}

	return imageCache_.data();
}

const coordinates_t* InputSet::coordinates() const {
	// If there is no cache
	if(coordinatesCache_.empty()) {
		std::vector<coordinates_t> data;
		coordinatesCache(data);
		coordinatesCache_.swap(data);
	}

	return coordinatesCache_.data();
}

const unsigned int* InputSet::heuristics() const {
	// If there is no cache
	if(heuristicCache_.empty()) {
		std::vector<unsigned int> data;
		heuristicCache(data);
		heuristicCache_.swap(data);
	}

	return heuristicCache_.data();
}

void InputSet::swapFeatures(std::vector<scalar_t>& data, bool flag) {
	// Compute the current matrix of features if not done already
	if(flag) {
		features();
	}

	// Swap the matrix cache
	matrixCache_.swap(data);
}

void InputSet::swapSamples(std::vector<scalar_t>& data, bool flag) {
	// Compute the current matrix of features if not done already
	if(flag) {
		samples();
	}

	// Swap the matrix cache
	matrixCache_.swapT(data);
}

void InputSet::swapLabels(std::vector<unsigned int>& data, bool flag) {
	// Compute the current vector of labels if not done already
	if(flag) {
		labels();
	}

	// Set the new number of labels
	if(!data.empty()) {
		nbLabelStack_.back() = *std::max_element(data.begin(), data.end()) + 1;
	}
	else {
		nbLabelStack_.back() = *(nbLabelStack_.end() - 2);
	}

	// Swap the label cache
	labelCache_.swap(data);
}

void InputSet::swapWeights(std::vector<scalar_t>& data, bool flag) {
	// Compute the current vector of weights if not done already
	if(flag) {
		weights();
	}

	// Swap the weight cache
	weightCache_.swap(data);
}

void InputSet::swapImages(std::vector<unsigned int>& data, bool flag) {
	// Compute the current vector of images if not done already
	if(flag) {
		images();
	}

	// Clear the matrix cache
	if(!data.empty()) {
		std::vector<scalar_t> empty;
		matrixCache_.swap(empty);
	}

	// Swap the image cache
	imageCache_.swap(data);
}

void InputSet::swapCoordinates(std::vector<coordinates_t>& data, bool flag) {
	// Compute the current vector of weights if not done already
	if(flag) {
		coordinates();
	}

	// Clear the matrix cache
	if(!data.empty()) {
		std::vector<scalar_t> empty;
		matrixCache_.swap(empty);
	}

	// Swap the coordinates cache
	coordinatesCache_.swap(data);
}

void InputSet::swapHeuristics(std::vector<unsigned int>& data, bool flag) {
	// Compute the current vector of heuristics if not done already
	if(flag) {
		heuristics();
	}

	// Swap the heuristic cache
	heuristicCache_.swap(data);
}

void InputSet::clear() {
	// Clear all the caches
	matrixCache_.clear();
	labelCache_.clear();
	weightCache_.clear();
	imageCache_.clear();
	coordinatesCache_.clear();
	heuristicCache_.clear();

	// Reset the number of labels
	for(unsigned int i = 1; i < nbLabelStack_.size(); ++i) {
		nbLabelStack_[i] = nbLabelStack_[0];
	}
}

struct sizeComparator {
	bool operator()(const std::vector<unsigned int>& a,
					const std::vector<unsigned int>& b) const {
		return a.size() < b.size();
	}
};

void InputSet::sampleFeatures(unsigned int nbTotal,
							  std::vector<unsigned int>& indices) {
	// Get the number of features and heuristics
	const unsigned int nbFeatures = this->nbFeatures();
	const unsigned int nbHeuristics = this->nbHeuristics();

	// Early return if the input set contains less or the same number of
	// features than the total requested
	if(nbFeatures <= nbTotal) {
		return;
	}

	// Get the heuristic associated to every feature
	const unsigned int* heuristics = this->heuristics();

	// Separate the features by heuristic
	std::vector<std::vector<unsigned int> > features(nbHeuristics);

	for(unsigned int f = 0; f < nbFeatures; ++f) {
		features[heuristics[f]].push_back(f);
	}

	// Sort the vectors by length
	std::sort(features.begin(), features.end(), sizeComparator());

	// The number of features to select from every heuristic
	const unsigned int nbToSelect = nbTotal / nbHeuristics;

	// The number of features that were saved so far and can be redistributed to
	// the remaining heuristics
	unsigned int nbSaved = 0;

	// Process the heuristics from the smallest to the largest
	for(unsigned int h = 0; h < nbHeuristics; ++h) {
		if(features[h].size() <= nbToSelect) {
			// Simply add all the features of the heuristic
			indices.insert(indices.end(), features[h].begin(),
						   features[h].end());

			// Redistribute the unused features
			nbSaved += nbToSelect - features[h].size();
		}
		else {
			// The number of features that were saved by the previous
			// heuristics and can be used by this one
			const unsigned int redistributable = nbSaved / (nbHeuristics - h);

			// The total number of features this heuristic should add
			unsigned int required = nbToSelect + redistributable;

			for(unsigned int f = 0; f < features[h].size(); ++f) {
				if((std::rand() % (features[h].size() - f)) < required) {
					indices.push_back(features[h][f]);
					--required;
				}
			}

			// Remove the redistributed features
			nbSaved -= redistributable;
		}
	}

	// Sort the features so that binary search can be used
	std::sort(indices.begin(), indices.end());

	// Add the remaining (nbTotal - indices.size()) features
	while(indices.size() < nbTotal) {
		unsigned int f;

		do {
			// Pick a (non-empty) heuristic at random
			unsigned int h;

			do {
				h = std::rand() % nbHeuristics;
			}
			while(features[h].empty());

			// Pick a feature at random in that heuristic
			f = features[h][std::rand() % features[h].size()];
		} while(*std::lower_bound(indices.begin(), indices.end(), f) == f);

		indices.insert(std::lower_bound(indices.begin(), indices.end(), f), f);
	}
}
