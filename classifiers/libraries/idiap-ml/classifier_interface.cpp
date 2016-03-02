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


/// \file	classifier_interface.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	Mar 21, 2011

#include "classifier_interface.h"

#include <algorithm>
#include <stdexcept>

using namespace ml;

unsigned int IClassifier::classify(const IDataSet& dataset,
								   unsigned int sample) const {
	// Assume that the distribution method is implemented
	std::vector<double> distr(dataset.nbLabels());
	distribution(dataset, sample, &distr[0]);

	// Returns the index of the class with the highest probability
	std::size_t index = std::max_element(distr.begin(), distr.end()) -
						distr.begin();

	return static_cast<unsigned int>(index);
}

void IClassifier::distribution(const IDataSet& dataset,
							   unsigned int sample,
							   double* distr) const {
	// Fill the distribution with zeros
	std::fill_n(distr, dataset.nbLabels(), 0.0);

	// Assume that the classify method is implemented
	unsigned int label = classify(dataset, sample);

	// Set label coefficient to 1
	if (label >= dataset.nbLabels()) {
		throw std::out_of_range("label is out of range");
	}

	distr[label] = 1.0;
}

void IClassifier::load(const json::Value& value) {
	throw std::logic_error("the classifier does not implement the load method");
}

void IClassifier::save(json::Value& value) const {
	throw std::logic_error("the classifier does not implement the save method");
}

void IClassifier::print(std::ostream& out) const {
	// Nothing to do
}
