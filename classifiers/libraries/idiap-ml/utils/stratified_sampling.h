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


/// \file	stratified_sampling.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date	May 5, 2011

#ifndef STRATIFIED_SAMPLING_H
#define STRATIFIED_SAMPLING_H

#include <cstdlib>

inline void stratifiedSampling(const double* weights, unsigned int nbWeights,
							   unsigned int* indices, unsigned int nbIndices) {
	long double sum = 0.0;

	for (unsigned int w = 0; w < nbWeights; ++w) {
		sum += weights[w];
	}

	const double q = sum / nbIndices;

	sum = 0.0;

	for (unsigned int i = 0, w = 0; i < nbIndices; ++i) {
		const double r = (i + drand48()) * q;

		while (sum <= r) {
			sum += weights[w];
			++w;
		}

		indices[i] = w - 1;
	}
}

#endif // STRATIFIED_SAMPLING_H
