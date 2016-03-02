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
/// @file MLClassifiers/Utils.cpp
/// @author Charles Dubout (charles.dubout@idiap.ch)
/// @date 2010.01.25
/// @version 1.5
//------------------------------------------------------------------------------

#include "Utils.h"

#include <algorithm>
#include <cstdlib>
#include <limits>

using namespace ML;

// Get the epsilon value of type scalar_t
const scalar_t Utils::epsilon = 10 * std::numeric_limits<scalar_t>::epsilon();

// Get the infinity value of type scalar_t
const scalar_t Utils::infinity = std::numeric_limits<scalar_t>::infinity();

//------------------------------------------------------------------------------
/// @brief	Comparator used to sort a vector of indices based on the values that
///			they index in a scalar_t vector
//------------------------------------------------------------------------------
class indicesComparator {
public:
	indicesComparator(const scalar_t* values) : values_(values) {
		// Nothing to do
	}

	bool operator()(unsigned int a, unsigned int b) const {
		return values_[a] < values_[b];
	}

private:
	const scalar_t* const values_;	///< The scalar_t vector of values
};

void Utils::sort(unsigned int* indices,
				 const scalar_t* values,
				 unsigned int nbSamples) {
	std::sort(indices, indices + nbSamples, indicesComparator(values));
}

//------------------------------------------------------------------------------
/// @brief	Predicate used to partition a vector of indices depending if the
///			value that they index is lower or equal (<=) or greater (>) than a
///			@p pivot
//------------------------------------------------------------------------------
class indexLessEqualPivot {
public:
	indexLessEqualPivot(const scalar_t* values,
						scalar_t pivot)
	: values_(values), pivot_(pivot) {
		// Nothing to do
	}

	bool operator()(unsigned int a) const {
		return values_[a] <= pivot_;
	}

private:
	const scalar_t* const values_;	///< The vector of values
	const scalar_t pivot_;			///< The pivot
};

unsigned int Utils::partition(unsigned int* indices,
							  const scalar_t* values,
							  unsigned int nbSamples,
							  scalar_t pivot) {
	return std::partition(indices, indices + nbSamples,
						  indexLessEqualPivot(values, pivot)) - indices;
}

void Utils::robustSampling(const scalar_t* weights,
						   unsigned int nbWeights,
						   unsigned int* indices,
						   unsigned int nbIndices) {
	if(nbWeights == 1) {
		std::fill_n(indices, nbIndices, 0);
	}
	else {
		scalar_t* pairWeights = new scalar_t[(nbWeights + 1) / 2];

		unsigned int s;

		for(s = 0; s < nbWeights / 2; ++s) {
			pairWeights[s] = weights[2 * s] + weights[2 * s + 1];
		}

		if(nbWeights & 1) {
			pairWeights[s] = weights[nbWeights - 1];
		}

		robustSampling(pairWeights, (nbWeights + 1) / 2, indices, nbIndices);

		for(unsigned int i = 0; i < nbIndices; ++i) {
			s = indices[i];
			// There is a bit of a trick for the isolated sample in the odd
			// case. Since the corresponding pair weight is the same as the
			// one sample alone, the test is always true and the isolated
			// sample will be taken for sure.
			if(drand48() * pairWeights[s] <= weights[2 * s]) {
				indices[i] = 2 * s;
			}
			else {
				indices[i] = 2 * s + 1;
			}
		}

		delete[] pairWeights;
	}
}
