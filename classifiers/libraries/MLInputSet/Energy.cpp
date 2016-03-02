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
/// @file MLInputSet/Energy.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.07.01
/// @version 1.5
//------------------------------------------------------------------------------

#include "Energy.h"

#include <cmath>

using namespace ML;

Energy::Energy(InputSet& inputSet, unsigned int k) : Filter(inputSet), k_(k) {
	// Compute the norms of the samples
	computeNorms();
}

void Energy::matrixCache(std::vector<scalar_t>& data) const {
	// Aquire the raw data
	Filter::matrixCache(data);

	// Get the current number of samples and features
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbFeatures = this->nbFeatures();

	// Find the indices of the currently selected samples
	std::vector<unsigned int> indices(nbSamples);

	for(unsigned int s = 0; s < nbSamples; ++s) {
		indices[s] = s;

		for(unsigned int i = sampleStack_.size() - 1; i > 0; --i) {
			indices[s] = sampleStack_[i][indices[s]];
		}
	}

	// Apply the filter on the data
	for(unsigned int s = 0; s < nbSamples; ++s) {
		for(unsigned int f = 0; f < nbFeatures; ++f) {
			data[s * nbFeatures + f] /= norms_[indices[s]];
		}
	}
}

void Energy::computeNorms() {
	// Get the current number of samples and features
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbFeatures = this->nbFeatures();

	// The norms of the samples
	norms_.clear();
	norms_.resize(nbSamples);

	for(unsigned int s = 0; s < nbSamples; ++s) {
		std::vector<unsigned int> sample(1, s);
		inputSet_.pushSamples(sample);

		const scalar_t* features = inputSet_.features(0);

		// Use a double for improved precision
		double norm = 0;

		for(unsigned int f = 0; f < nbFeatures; ++f) {
			norm += std::pow(double(std::abs(features[f])), double(k_));
		}

		norms_[s] = (k_ > 1) ? std::pow(norm, 1.0 / k_) : norm;

		inputSet_.popSamples();
	}
}
