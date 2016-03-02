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
/// @file MLInputSet/Uniform.h
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.06.17
/// @version 1.5
//------------------------------------------------------------------------------

#include "Uniform.h"

#include <cassert>

using namespace ML;

Uniform::Uniform(InputSet& inputSet,
				 unsigned int nbTotalFeatures) : Filter(inputSet) {
	// Make sure that nbTotalFeatures is strictly positive
	assert(nbTotalFeatures);

	// Get the number of samples and features as well as the heuristics
	const unsigned int nbSamples = inputSet.nbSamples();
	const unsigned int nbFeatures = inputSet.nbFeatures();
	const unsigned int* heuristics = inputSet.heuristics();

	// Separate the features of every heuristic
	std::vector<std::vector<unsigned int> > features(nbHeuristics_);

	for(unsigned int f = 0; f < nbFeatures; ++f) {
		features[heuristics[f]].push_back(f);
	}

	// For every desired feature, first sample one heuristic at random, then a
	// random features in that heuristic
	std::vector<unsigned int> featureStack;

	for(unsigned int f = 0; f < nbTotalFeatures; ++f) {
		// Select a (non-empty) heuristic at random
		unsigned int h;

		do {
			h = std::rand() % nbHeuristics_;
		}
		while(features[h].empty());

		// Select a features of that heuristic at random and add it to the stack
		unsigned int g = std::rand() % features[h].size();
		featureStack.push_back(features[h][g]);
		features[h].erase(features[h].begin() + g);
	}

	// Push the selected features
	featureStack_.push_back(featureStack);
}
