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
/// @file MLInputSet/MinMax.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.06.01
/// @version 1.5
//------------------------------------------------------------------------------

#include "MinMax.h"

#include <cassert>
#include <limits>

using namespace ML;

MinMax::MinMax(InputSet& inputSet) : Filter(inputSet) {
	// Get the number of samples and features
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbFeatures = inputSet.nbFeatures();

	// Used to store the current min and max of every feature
	mins_.resize(nbFeatures, std::numeric_limits<scalar_t>::infinity());
	maxs_.resize(nbFeatures,-std::numeric_limits<scalar_t>::infinity());

	for(unsigned int s = 0; s < nbSamples; ++s) {
		std::vector<unsigned int> sample(1, s);
		inputSet.pushSamples(sample);

		const scalar_t* features = inputSet.features(0);

		for(unsigned int f = 0; f < nbFeatures; ++f) {
			if(features[f] < mins_[f]) {
				mins_[f] = features[f];
			}

			if(features[f] > maxs_[f]) {
				maxs_[f] = features[f];
			}
		}

		inputSet.popSamples();
	}

	std::vector<unsigned int> featureStack;

	for(unsigned int f = 0; f < nbFeatures; ++f) {
		if(mins_[f] < maxs_[f]) {
			featureStack.push_back(f);
		}
	}

	// There must be at least one feature
	assert(!featureStack.empty());

	// Push the selected features if needed
	if(featureStack.size() < nbFeatures) {
		featureStack_.push_back(featureStack);
	}
}

void MinMax::matrixCache(std::vector<scalar_t>& data) const {
	// Aquire the raw data
	Filter::matrixCache(data);

	// Get the current number of samples and features
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbFeatures = this->nbFeatures();

	// Find the indices of the currently selected features
	std::vector<unsigned int> indices(nbFeatures);

	for(unsigned int f = 0; f < nbFeatures; ++f) {
		indices[f] = f;

		for(unsigned int i = featureStack_.size() - 1; i > 0; --i) {
			indices[f] = featureStack_[i][indices[f]];
		}
	}

	// Apply the filter on the data
	for(unsigned int s = 0; s < nbSamples; ++s) {
		for(unsigned int f = 0; f < nbFeatures; ++f) {
			data[s * nbFeatures + f] =
				(data[s * nbFeatures + f] - mins_[indices[f]]) /
				(maxs_[indices[f]] - mins_[indices[f]]);
		}
	}
}
