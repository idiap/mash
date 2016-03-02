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


/// \file	robust_sampling.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	May 4, 2011

#ifndef ROBUST_SAMPLING_H
#define ROBUST_SAMPLING_H

#include <algorithm>
#include <cstdlib>
#include <vector>

inline void robustSampling(const double* weights,
						   unsigned int nbWeights,
						   unsigned int* indices,
						   unsigned int nbIndices) {
	if(nbWeights == 1) {
		std::fill_n(indices, nbIndices, 0);
	}
	else {
		std::vector<double> pairWeights((nbWeights + 1) >> 1);

		for(unsigned int s = 0; s < (nbWeights >> 1); ++s) {
			pairWeights[s] = weights[s << 1] + weights[(s << 1) + 1];
		}

		if(nbWeights & 1) {
			pairWeights[nbWeights >> 1] = weights[nbWeights - 1];
		}

		robustSampling(&pairWeights[0], (nbWeights + 1) >> 1, indices, nbIndices);

		for(unsigned int i = 0; i < nbIndices; ++i) {
			// There is a bit of a trick for the isolated sample in the odd
			// case. Since the corresponding pair weight is the same as the
			// one sample alone, the test is always true and the isolated
			// sample will be taken for sure.
			indices[i] = (indices[i] << 1) +
						 (drand48() * pairWeights[indices[i]] > weights[indices[i] << 1]);
		}
	}
}

#endif // ROBUST_SAMPLING_H
