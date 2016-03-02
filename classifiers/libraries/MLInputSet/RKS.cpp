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
/// @file MLInputSet/RKS.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.06.01
/// @version 1.5
//------------------------------------------------------------------------------

#include "RKS.h"

#include <Eigen/Core>

#include <cassert>
#include <cmath>

using namespace ML;

typedef Eigen::Map<Eigen::Matrix<scalar_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > Matrix;

RKS::RKS(InputSet& inputSet,
		 scalar_t gamma,
		 unsigned int nbProjections)
: Filter(inputSet), nbProjections_(nbProjections), bias_(nbProjections),
  cached_(false) {
	// Gamma must be strictly positive
	assert(gamma > 0);

	// The number of projections must be strictly positive
	assert(nbProjections);

	// Get the number of samples and features
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbFeatures = inputSet.nbFeatures();

	// Draw the projections at random
	projections_.clear();
	projections_.resize(nbFeatures * nbProjections_);

	const scalar_t sqrtGamma = std::sqrt(gamma / (2 * scalar_t(M_PI)));

	for(unsigned int f = 0; f < projections_.size(); ++f) {
		projections_[f] = sqrtGamma * randn();
	}

	// Draw the bias at random
	for(unsigned int p = 0; p < nbProjections_; ++p) {
		bias_[p] = 2 * scalar_t(M_PI) * randn();
	}

	// Push as many features as there is projections
	std::vector<unsigned int> featureStack(nbProjections);

	for(unsigned int i = 0; i < nbProjections; ++i) {
		featureStack[i] = i;
	}

	featureStack_.push_back(featureStack);
}

void RKS::matrixCache(std::vector<scalar_t>& data) const {
	// We need all the features
	std::vector<std::vector<unsigned int> > tmp(1);
	std::vector<std::vector<unsigned int> >& featureStack =
		const_cast<std::vector<std::vector<unsigned int> >&>(featureStack_);

	featureStack.swap(tmp);
	tmp[0].swap(featureStack[0]);

	// Aquire the raw data
	Filter::matrixCache(data);

	// Number of features of the underlying input set
	const unsigned int nbUnderlying = this->nbFeatures();

	// Restore the feature stack
	featureStack[0].swap(tmp[0]);
	tmp.swap(featureStack);

	// Get the current number of samples and features
	const unsigned int nbSamples = this->nbSamples();
	const unsigned int nbFeatures = this->nbFeatures();

	// Temporary matrix used to hold the result of the projection
	std::vector<scalar_t> proj(nbSamples * nbProjections_);
	Matrix matProj(&proj[0], nbSamples, nbProjections_);

	// Easiest case, just do a matrix multiplication
	if(nbFeatures == nbProjections_) {
		assert(data.size() == nbSamples * nbUnderlying);
		assert(projections_.size() == nbUnderlying * nbProjections_);
		Matrix matProj(&proj[0], nbSamples, nbProjections_);
		matProj = Matrix(&data[0], nbSamples, nbUnderlying) *
				  Matrix(&projections_[0], nbUnderlying, nbProjections_);

		for(unsigned int s = 0; s < nbSamples; ++s) {
			for(unsigned int p = 0; p < nbProjections_; ++p) {
				proj[s * nbProjections_ + p] =
					std::cos(proj[s * nbProjections_ + p] + bias_[p]);
			}
		}
	}
	// Find the currently selected features
	else {
		for(unsigned int f = 0; f < nbFeatures; ++f) {
			register unsigned int index = f;

			for(unsigned int i = featureStack_.size() - 1; i > 0; --i) {
				index = featureStack_[i][index];
			}

			matProj.col(f) =
				Matrix(&data[0], nbSamples, nbFeatures) *
				Matrix(&projections_[0], nbFeatures, nbProjections_).col(index);

			for(unsigned int s = 0; s < nbSamples; ++s) {
				proj[s * nbFeatures + f] =
					std::cos(proj[s * nbFeatures + f] + bias_[index]);
			}
		}
	}

	data.swap(proj);
}

void RKS::heuristicCache(std::vector<unsigned int>& data) const {
	data.clear();
	data.resize(nbFeatures());
}

// Return normaly distributed random value
scalar_t RKS::randn() const {
	// Return the cached value if there is one
	if(cached_) {
		cached_ = false;
		return tmp_;
	}

	scalar_t x, y, r;

	do {
		x = std::rand() / scalar_t(RAND_MAX) * 2 - 1;
		y = std::rand() / scalar_t(RAND_MAX) * 2 - 1;
		r = x * x + y * y;
	}
	while(r > 1 || r <= 0);

	// Cache the first value
	scalar_t s = std::sqrt(-2 * std::log(r) / r);
	tmp_ = x * s;
	cached_ = true;

	// Return the second value
	return y * s;
}
