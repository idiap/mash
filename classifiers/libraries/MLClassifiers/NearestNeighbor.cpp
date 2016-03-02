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
/// @file MLClassifiers/NearestNeighbor.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.03.12
/// @version 2.1
//------------------------------------------------------------------------------

#include "NearestNeighbor.h"

#include <cmath>
#include <iostream>
#include <numeric>
#include <limits>
#include <stdexcept>

using namespace ML;

NearestNeighbor::NearestNeighbor(unsigned int nbNeighbors,
								 unsigned int Lk)
: nbNeighbors_(nbNeighbors), Lk_(Lk) {
	if(nbNeighbors < 1) {
		nbNeighbors = 1;
	}
}

Classifier* NearestNeighbor::clone() const {
	return new NearestNeighbor(*this);
}

void NearestNeighbor::train(InputSet& inputSet) {
	// 'Steal' the features of the input set
	matrix_.clear();
	inputSet.swapFeatures(matrix_);

	// 'Steal' the labels of the input set
	labels_.clear();
	inputSet.swapLabels(labels_);
}

// Fast exponentiation function
inline scalar_t powi(scalar_t base, unsigned int exponent) {
	scalar_t ret = 1;

	while(exponent) {
		if(exponent & 1) {
			ret *= base;
		}

		base *= base;
		exponent >>= 1;
	}

	return ret;
}

unsigned int NearestNeighbor::classify(InputSet& inputSet,
									   unsigned int sample) const {
	// Get the number of samples, features and labels
	const unsigned int nbSamples = labels_.size();
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int nbLabels = inputSet.nbLabels();

	// Look for the nbNeighbors samples in the training set with the smallest
	// L^k norm distance from the given sample

	// Get the features of the sample to classify
	const scalar_t* features = inputSet.features(sample);

	// Create pairs of distance/index for the candidate neighbors
	std::vector<std::pair<scalar_t, unsigned int> >
	neighbors(nbNeighbors_,
			  std::make_pair(std::numeric_limits<scalar_t>::infinity(), 0U));

	for(unsigned int s = 0; s < nbSamples; ++s) {
		// Compute the L^k distance between s and the sample to classifiy
		double d = 0;

		for(unsigned int f = 0; f < nbFeatures; ++f) {
		//	d += std::pow(std::abs(features[f] - matrix_[s * nbFeatures + f]), Lk_);
			d += powi(std::abs(features[f] - matrix_[s * nbFeatures + f]), Lk_);
		}

		// If it is closer than the last candidate so far
		if(d < neighbors.back().first) {
			std::vector<std::pair<scalar_t, unsigned int> >::iterator p =
				std::upper_bound(neighbors.begin(), neighbors.end(),
								 std::make_pair(scalar_t(d), 0U));

			if(p != neighbors.end()) {
				std::copy_backward(p, --neighbors.end(), neighbors.end());
				p->first  = d;
				p->second = s;
			}
		}
	}

	// Return the most frequent label
	std::vector<unsigned int> frequencies(nbLabels, 0);

	for(unsigned int i = 0; i < nbNeighbors_; ++i) {
		++frequencies[labels_[neighbors[i].second]];
	}

	return std::max_element(frequencies.begin(), frequencies.end()) -
		   frequencies.begin();
}

void NearestNeighbor::print(std::ostream& out) const {
	out << "Nearest neighbor classifier (" << nbNeighbors_ << " neighbors, L^"
		<< Lk_ << " norm)." << std::endl;
}
