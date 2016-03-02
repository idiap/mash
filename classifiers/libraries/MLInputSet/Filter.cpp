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
/// @file MLInputSet/Filter.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.05.31
/// @version 1.5
//------------------------------------------------------------------------------

#include "Filter.h"

#include <cassert>

using namespace ML;

Filter::Filter(InputSet& inputSet) : inputSet_(inputSet) {
	// Get the input set's number of samples, features and labels
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Resize the initial stack of samples
	sampleStack_.resize(1);
	sampleStack_[0].resize(nbSamples);

	// Make the indices range from 0 to nbSamples - 1
	for(unsigned int s = 0; s < nbSamples; ++s) {
		sampleStack_[0][s] = s;
	}

	// Resize the initial stack of features
	featureStack_.resize(1);
	featureStack_[0].resize(nbFeatures);

	// Make the indices range from 0 to nbFeatures - 1
	for(unsigned int f = 0; f < nbFeatures; ++f) {
		featureStack_[0][f] = f;
	}

	// Set the number of labels
	nbLabelStack_.push_back(nbLabels); // Push it twice so as to be sure to
	nbLabelStack_.push_back(nbLabels); // never overwrite it

	// Set the number of images and heuristics
	nbImages_ = inputSet.nbImages();
	nbHeuristics_ = inputSet.nbHeuristics();
}

void Filter::convert(std::vector<unsigned int>& features) const {
	for(unsigned int f = 0; f < features.size(); ++f) {
		for(unsigned int i = featureStack_.size() - 1; i > 0; --i) {
			features[f] = featureStack_[i][features[f]];
		}
	}
}

void Filter::clear() {
	InputSet::clear();
	inputSet_.clear();
}

void Filter::matrixCache(std::vector<scalar_t>& data) const {
	// Get the current number of samples and features
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbFeatures = this->nbFeatures();

	// Push the currently selected samples on the input set if needed
	if(sampleStack_.size() > 1) {
		std::vector<unsigned int> indices(nbSamples);

		for(unsigned int s = 0; s < nbSamples; ++s) {
			indices[s] = s;

			for(unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
				indices[s] = sampleStack_[i][indices[s]];
			}
		}

		inputSet_.pushSamples(indices);
	}

	// Push the currently selected features on the input set if needed
	if(featureStack_.size() > 1) {
		std::vector<unsigned int> indices(nbFeatures);

		for(unsigned int f = 0; f < nbFeatures; ++f) {
			indices[f] = f;

			for(unsigned int i = featureStack_.size() - 1; i > 0; --i) {
				indices[f] = featureStack_[i][indices[f]];
			}
		}

		inputSet_.pushFeatures(indices);
	}

	// Swap the image cache as it is used by the matrixCache method of the
	// filtered input set
	std::vector<unsigned int> imageCache;
	images();
	imageCache_.swap(imageCache);
	inputSet_.swapImages(imageCache, false);

	// Swap the coordinates cache as it is used by the matrixCache method of the
	// filtered input set
	std::vector<coordinates_t> coordinatesCache;
	coordinates();
	coordinatesCache_.swap(coordinatesCache);
	inputSet_.swapCoordinates(coordinatesCache, false);

	// 'Steal' the cache
	inputSet_.swapFeatures(data);

	// Swap back the image cache
	inputSet_.swapImages(imageCache, false);
	imageCache_.swap(imageCache);

	// Swap back the coordinates cache
	inputSet_.swapCoordinates(coordinatesCache, false);	
	coordinatesCache_.swap(coordinatesCache);

	if(featureStack_.size() > 1) {
		inputSet_.popFeatures();
	}

	if(sampleStack_.size() > 1) {
		inputSet_.popSamples();
	}
}

void Filter::labelCache(std::vector<unsigned int>& data) const {
	// Get the current number of samples
	const unsigned int nbSamples = this->nbSamples();

	// Push the currently selected samples on the input set if needed
	if(sampleStack_.size() > 1) {
		std::vector<unsigned int> indices(nbSamples);

		for(unsigned int s = 0; s < nbSamples; ++s) {
			indices[s] = s;

			for(unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
				indices[s] = sampleStack_[i][indices[s]];
			}
		}

		inputSet_.pushSamples(indices);
	}

	// 'Steal' the cache
	inputSet_.swapLabels(data);

	if(sampleStack_.size() > 1) {
		inputSet_.popSamples();
	}
}

void Filter::weightCache(std::vector<scalar_t>& data) const {
	// Get the current number of samples
	const unsigned int nbSamples = this->nbSamples();

	// Push the currently selected samples on the input set if needed
	if(sampleStack_.size() > 1) {
		std::vector<unsigned int> indices(nbSamples);

		for(unsigned int s = 0; s < nbSamples; ++s) {
			indices[s] = s;

			for(unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
				indices[s] = sampleStack_[i][indices[s]];
			}
		}

		inputSet_.pushSamples(indices);
	}

	// 'Steal' the cache
	inputSet_.swapWeights(data);

	if(sampleStack_.size() > 1) {
		inputSet_.popSamples();
	}
}

void Filter::imageCache(std::vector<unsigned int>& data) const {
	// Get the current number of samples
	const unsigned int nbSamples = this->nbSamples();

	// Push the currently selected samples on the input set if needed
	if(sampleStack_.size() > 1) {
		std::vector<unsigned int> indices(nbSamples);

		for(unsigned int s = 0; s < nbSamples; ++s) {
			indices[s] = s;

			for(unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
				indices[s] = sampleStack_[i][indices[s]];
			}
		}

		inputSet_.pushSamples(indices);
	}

	// 'Steal' the cache
	inputSet_.swapImages(data);

	if(sampleStack_.size() > 1) {
		inputSet_.popSamples();
	}
}

void Filter::coordinatesCache(std::vector<coordinates_t>& data) const {
	// Get the current number of samples
	const unsigned int nbSamples = this->nbSamples();

	// Push the currently selected samples on the input set if needed
	if(sampleStack_.size() > 1) {
		std::vector<unsigned int> indices(nbSamples);

		for(unsigned int s = 0; s < nbSamples; ++s) {
			indices[s] = s;

			for(unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
				indices[s] = sampleStack_[i][indices[s]];
			}
		}

		inputSet_.pushSamples(indices);
	}

	// 'Steal' the cache
	inputSet_.swapCoordinates(data);

	if(sampleStack_.size() > 1) {
		inputSet_.popSamples();
	}
}

void Filter::heuristicCache(std::vector<unsigned int>& data) const {
	// Get the current number of features
	const unsigned int nbFeatures = this->nbFeatures();

	// Push the currently selected features on the input set if needed
	if(featureStack_.size() > 1) {
		std::vector<unsigned int> indices(nbFeatures);

		for(unsigned int s = 0; s < nbFeatures; ++s) {
			indices[s] = s;

			for(unsigned int i = featureStack_.size() - 1; i > 0; --i) {
				indices[s] = featureStack_[i][indices[s]];
			}
		}

		inputSet_.pushFeatures(indices);
	}

	// 'Steal' the cache
	inputSet_.swapHeuristics(data);

	if(featureStack_.size() > 1) {
		inputSet_.popFeatures();
	}
}
