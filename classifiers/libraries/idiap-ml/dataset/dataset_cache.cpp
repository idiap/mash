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


/// \file	dataset_cache.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 24, 2011

#include "dataset_cache.h"

#include <algorithm>
#include <stdexcept>

using namespace ml;

DataSetCache::DataSetCache(const IDataSet* dataset,
						   bool transpose)
: DataSetFilter(dataset), transpose_(transpose) {
	// Cache the whole dataset
	const unsigned int nbSamples = dataset->nbSamples();
	const unsigned int nbFeatures = dataset->nbFeatures();

	values_.resize(nbSamples * nbFeatures);

	std::vector<unsigned int> indices(std::max(nbSamples, nbFeatures));

	for (unsigned int i = 0; i < std::max(nbSamples, nbFeatures); ++i) {
		indices[i] = i;
	}

	dataset->computeFeatures(nbSamples, &indices[0], nbFeatures, &indices[0],
							 &values_[0], transpose);

	labels_.resize(nbSamples);

	for (unsigned int s = 0; s < nbSamples; ++s) {
		labels_[s] = dataset->label(s);
	}

	heuristics_.resize(nbFeatures);

	for (unsigned int f = 0; f < nbFeatures; ++f) {
		heuristics_[f] = dataset->heuristic(f);
	}
}

void DataSetCache::computeFeatures(unsigned int nbSamples,
								   const unsigned int* sampleIndices,
								   unsigned int nbFeatures,
								   const unsigned int* featureIndices,
								   float* values,
								   bool transpose) const {
	// Make sure that the indices are not 0
	if (!nbSamples || !sampleIndices || !nbFeatures || !featureIndices) {
		throw std::invalid_argument("no sample or feature indices provided");
	}

	// The stride of the matrix
	const unsigned int stride = transpose_ ? this->nbSamples() :
											 this->nbFeatures();

#ifndef NDEBUG
	for (unsigned int s = 0; s < nbSamples; ++s) {
		if (sampleIndices[s] >= this->nbSamples()) {
			throw std::out_of_range("out of range sample");
		}
	}

	for (unsigned int f = 0; f < nbFeatures; ++f) {
		if (featureIndices[f] >= this->nbFeatures()) {
			throw std::out_of_range("out of range feature");
		}
	}
#endif

	if (!transpose && !transpose_) {
		for (unsigned int s = 0; s < nbSamples; ++s) {
			const unsigned int offset0 = s * nbFeatures;
			const unsigned int offset1 = sampleIndices[s] * stride;

			for (unsigned int f = 0; f < nbFeatures; ++f) {
				values[offset0 + f] = values_[offset1 + featureIndices[f]];
			}
		}
	}
	else if (!transpose && transpose_) {
		for (unsigned int s = 0; s < nbSamples; ++s) {
			const unsigned int offset0 = s * nbFeatures;
			const unsigned int offset1 = sampleIndices[s];

			for (unsigned int f = 0; f < nbFeatures; ++f) {
				values[offset0 + f] = values_[offset1 + featureIndices[f] * stride];
			}
		}
	}
	else if (transpose && !transpose_) {
		for (unsigned int f = 0; f < nbFeatures; ++f) {
			const unsigned int offset0 = f * nbSamples;
			const unsigned int offset1 = featureIndices[f];

			for (unsigned int s = 0; s < nbSamples; ++s) {
				values[offset0 + s] = values_[offset1 + sampleIndices[s] * stride];
			}
		}
	}
	else { // if (transpose && transpose_) {
		for (unsigned int f = 0; f < nbFeatures; ++f) {
			const unsigned int offset0 = f * nbSamples;
			const unsigned int offset1 = featureIndices[f] * stride;

			for (unsigned int s = 0; s < nbSamples; ++s) {
				values[offset0 + s] = values_[offset1 + sampleIndices[s]];
			}
		}
	}
}

unsigned int DataSetCache::label(unsigned int sample) const {
#ifndef NDEBUG
	return labels_.at(sample);
#else
	return labels_[sample];
#endif
}

unsigned int DataSetCache::heuristic(unsigned int feature) const {
#ifndef NDEBUG
	return heuristics_.at(feature);
#else
	return heuristics_[feature];
#endif
}
