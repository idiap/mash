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


/// \file	dataset_filter.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 24, 2011

#include "dataset_filter.h"

#include <stdexcept>

using namespace ml;

DataSetFilter::DataSetFilter(const IDataSet* dataset) : dataset_(dataset) {
	// Make sure that the dataset is not 0
	if (!dataset) {
		throw std::invalid_argument("no dataset provided");
	}
}

DataSetFilter::~DataSetFilter() {
	// Nothing to do
}

unsigned int DataSetFilter::nbSamples() const {
	return dataset_->nbSamples();
}

unsigned int DataSetFilter::nbFeatures() const {
	return dataset_->nbFeatures();
}

unsigned int DataSetFilter::nbLabels() const {
	return dataset_->nbLabels();
}

unsigned int DataSetFilter::nbHeuristics() const {
	return dataset_->nbHeuristics();
}

void DataSetFilter::computeFeatures(unsigned int nbSamples,
									const unsigned int* sampleIndices,
									unsigned int nbFeatures,
									const unsigned int* featureIndices,
									float* values,
									bool transpose) const {
	dataset_->computeFeatures(nbSamples, sampleIndices, nbFeatures,
							  featureIndices, values, transpose);
}

unsigned int DataSetFilter::label(unsigned int sample) const {
	return dataset_->label(sample);
}

unsigned int DataSetFilter::heuristic(unsigned int feature) const {
	return dataset_->heuristic(feature);
}

void DataSetFilter::filter(const IDataSet* dataset) {
	dataset_ = dataset;
}

