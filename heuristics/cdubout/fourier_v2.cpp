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


/** Author: Charles Dubout (charles.dubout@idiap.ch)
 
 Fourier transform heuristic. Compute the 2D DFT of the region of interest to
 convert it to the frequency domain.
 */

#include <mash/heuristic.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'Fourier' heuristic class
//------------------------------------------------------------------------------
class FourierHeuristic : public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();
	virtual void prepareForCoordinates();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// Type to represent a complex number
	typedef std::pair<scalar_t, scalar_t> Complex;

	// Round up to the next highest power of 2
	static unsigned int roundUp(unsigned int x);

	// Fast fourier transform
	static void fft(scalar_t* data, unsigned int n);

	// The transformed version of the region of interest
	std::vector<Complex> features_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new FourierHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int FourierHeuristic::dim() {
	// Twice the number of pixels (rounded to the next highest power of 2)
	unsigned int roi_size = roi_extent * 2 + 1;
	unsigned int roi_size_2 = roundUp(roi_size);
	return 2 * roi_size_2 * roi_size_2;
}

void FourierHeuristic::prepareForCoordinates() {
	// Compute the coordinates of the top-left pixel of the region of interest
	unsigned int x0 = coordinates.x - roi_extent;
	unsigned int y0 = coordinates.y - roi_extent;

	// Compute the size of the region of interest
	unsigned int roi_size = roi_extent * 2 + 1;
	unsigned int roi_size_2 = roundUp(roi_size);

	// Get the pixels values of the region of interest
	byte_t** pLines = image->grayLines();

	// Resize the features to the correct dimension
	features_.clear();
	features_.resize(dim() / 2); // Complex is a pair of scalar_t

	// Fill the feature image
	for(unsigned int y = 0; y < roi_size; ++y) {
		for(unsigned int x = 0; x < roi_size; ++x) {
			features_[y * roi_size_2 + x].first = pLines[y][x];
		}
	}

	// Do 1D ffts along the lines of the region of interest
	for(unsigned int y = 0; y < roi_size; ++y) {
		fft((scalar_t*)&features_[y * roi_size_2], roi_size_2);
	}

	// Do 1D ffts along the columns of the region of interest
	std::vector<Complex> tmp(roi_size_2);

	for(unsigned int x = 0; x < roi_size_2; ++x) {
		for(unsigned int y = 0; y < roi_size_2; ++y) {
			tmp[y] = features_[y * roi_size_2 + x];
		}

		fft((scalar_t*)&tmp[0], roi_size_2);

		for(unsigned int y = 0; y < roi_size_2; ++y) {
			features_[y * roi_size_2 + x] = tmp[y];
		}
	}
}

scalar_t FourierHeuristic::computeFeature(unsigned int feature_index) {
	// Separate the real and imaginary part of the image (not necessary)
	if(feature_index < features_.size()) {
		return features_[feature_index].first;
	}
	else {
		return features_[feature_index - features_.size()].second;
	}
}

// Round up to the next highest power of 2
unsigned int FourierHeuristic::roundUp(unsigned int x) {
	// Assume x is 32-bits
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return ++x;
}

// Fast fourier transform (adapted from the book Numerical Recipes, 3rd Ed.)
void FourierHeuristic::fft(scalar_t* data, unsigned int n) {
	const unsigned int nn = n << 1;

	// Bit-reversal section
	for(unsigned int i = 1, j = 1; i < nn; i += 2) {
		if(i < j) { // Exchange the 2 complex number
			std::swap(data[i - 1], data[j - 1]);
			std::swap(data[i], data[j]);
		}

		unsigned int m = n;

		while(m > 1 && j > m) {
			j -= m;
			m >>= 1;
		}

		j += m;
	}

	// Danielson-Lanczos section
	unsigned int mmax = 2;

	while(mmax < nn) {
		const unsigned int istep = mmax << 1;
		scalar_t theta = scalar_t(6.28318530717959) / mmax;
		scalar_t wtemp = std::sin(scalar_t(0.5) * theta);
		scalar_t wpr = -2 * wtemp * wtemp;
		scalar_t wpi = std::sin(theta);
		scalar_t wr = 1;
		scalar_t wi = 0;

		for(unsigned int m = 1; m < mmax; m += 2) {
			for(unsigned int i = m; i <= nn; i += istep) {
				unsigned int j = i + mmax;
				scalar_t tempr = wr * data[j - 1] - wi * data[j];
				scalar_t tempi = wr * data[j] + wi * data[j - 1];
				data[j - 1] = data[i - 1] - tempr;
				data[j] = data[i] - tempi;
				data[i - 1] += tempr;
				data[i] += tempi;
			}

			wtemp = wr;
			wr = wr * wpr - wi * wpi + wr;
			wi = wi * wpr + wtemp * wpi + wi;
		}

		mmax = istep;
	}
}
