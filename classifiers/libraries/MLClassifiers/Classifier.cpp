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
/// @file MLClassifiers/Classifier.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.02.04
/// @version 1.7
//------------------------------------------------------------------------------

#include "Classifier.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace ML;

Classifier::~Classifier() {
	// Nothing to do
}

unsigned int Classifier::classify(InputSet& inputSet,
								  unsigned int sample) const {
	// Assumes that the distribution method is implemented
	std::vector<scalar_t> distr(inputSet.nbLabels());
	distribution(inputSet, sample, &distr[0]);

	// Returns the index of the class with the highest probability
	return std::max_element(distr.begin(), distr.end()) - distr.begin();
}

void Classifier::distribution(InputSet& inputSet,
							  unsigned int sample,
							  scalar_t* distr) const {
	// Fill the distribution with zeros
	std::fill_n(distr, inputSet.nbLabels(), 0);

	// Assumes that the classify method is implemented
	unsigned int label = classify(inputSet, sample);

	// Set label coefficient to 1
	assert(label < inputSet.nbLabels());
	distr[label] = 1;
}

void Classifier::print(std::ostream& out) const {
	out << "Unknown classifier." << std::endl;
}

void Classifier::report(std::vector<unsigned int>&) const {
	// Nothing to do
}
