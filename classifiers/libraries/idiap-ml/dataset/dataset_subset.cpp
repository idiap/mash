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


/// \file	dataset_subset.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 24, 2011

#include "dataset_subset.h"

#include <stdexcept>

using namespace ml;

SampleSubSet::SampleSubSet(const IDataSet* dataset,
						   unsigned int nbSamples,
						   const unsigned int* sampleIndices)
: DataSetFilter(dataset),
  sampleIndices_(sampleIndices, sampleIndices + nbSamples) {
	// Make sure that the indices are not 0
	if (!nbSamples || !sampleIndices) {
		throw std::invalid_argument("no sample indices provided");
	}
}

unsigned int SampleSubSet::nbSamples() const {
	return static_cast<unsigned int>(sampleIndices_.size());
}

void SampleSubSet::computeFeatures(unsigned int nbSamples,
								   const unsigned int* sampleIndices,
								   unsigned int nbFeatures,
								   const unsigned int* featureIndices,
								   float* values,
								   bool transpose) const {
	// Make sure that the indices are not 0
	if (!nbSamples || !sampleIndices || !nbFeatures || !featureIndices) {
		throw std::invalid_argument("no sample or feature indices provided");
	}

	// Re-index the samples
	std::vector<unsigned int> samples(nbSamples);

	for (unsigned int s = 0; s < nbSamples; ++s) {
#ifndef NDEBUG
		samples[s] = sampleIndices_.at(sampleIndices[s]);
#else
		samples[s] = sampleIndices_[sampleIndices[s]];
#endif
	}

	dataset_->computeFeatures(nbSamples, &samples[0], nbFeatures, featureIndices,
							  values, transpose);
}

unsigned int SampleSubSet::label(unsigned int sample) const {
#ifndef NDEBUG
	return dataset_->label(sampleIndices_.at(sample));
#else
	return dataset_->label(sampleIndices_[sample]);
#endif
}

FeatureSubSet::FeatureSubSet(const IDataSet* dataset,
							 unsigned int nbFeatures,
							 const unsigned int* featureIndices)
: DataSetFilter(dataset),
  featureIndices_(featureIndices, featureIndices + nbFeatures) {
	// Make sure that the indices are not 0
	if (!nbFeatures || !featureIndices) {
		throw std::invalid_argument("no feature indices provided");
	}
}

unsigned int FeatureSubSet::nbFeatures() const {
	return static_cast<unsigned int>(featureIndices_.size());
}

void FeatureSubSet::computeFeatures(unsigned int nbSamples,
								    const unsigned int* sampleIndices,
								    unsigned int nbFeatures,
								    const unsigned int* featureIndices,
									float* values,
									bool transpose) const {
		// Make sure that the indices are not 0
	if (!nbSamples || !sampleIndices || !nbFeatures || !featureIndices) {
		throw std::invalid_argument("no sample or feature indices provided");
	}

	// Re-index the features
	std::vector<unsigned int> features(nbFeatures);

	for (unsigned int f = 0; f < nbFeatures; ++f) {
#ifndef NDEBUG
		features[f] = featureIndices_.at(featureIndices[f]);
#else
		features[f] = featureIndices_[featureIndices[f]];
#endif
	}

	dataset_->computeFeatures(nbSamples, sampleIndices, nbFeatures, &features[0],
							  values, transpose);
}

unsigned int FeatureSubSet::heuristic(unsigned int feature) const {
#ifndef NDBUG
	return dataset_->heuristic(featureIndices_.at(feature));
#else
	return dataset_->heuristic(featureIndices_[feature]);
#endif
}
