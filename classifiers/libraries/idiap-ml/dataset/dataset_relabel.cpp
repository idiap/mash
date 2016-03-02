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


/// \file	dataset_relabel.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Apr 28, 2011

#include "dataset_relabel.h"

#include <algorithm>
#include <stdexcept>

using namespace ml;

DataSetRelabel::DataSetRelabel(const IDataSet* dataset,
							   const unsigned int* map)
: DataSetFilter(dataset), map_(map, map + dataset->nbLabels()) {}

unsigned int DataSetRelabel::nbLabels() const {
	return *std::max_element(map_.begin(), map_.end()) + 1U;
}

unsigned int DataSetRelabel::label(unsigned int sample) const {
#ifndef NDEBUG
	return map_.at(dataset_->label(sample));
#else
	return map_[dataset_->label(sample)];
#endif
}
