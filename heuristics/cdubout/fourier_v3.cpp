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
#include <complex>
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
	// Round up to the next highest power of 2
	static unsigned int roundUp(unsigned int x);

	// Fast fourier transform
	static void fft(std::complex<scalar_t>* data,
					unsigned int n,
					unsigned int stride = 1,
					bool inverse = false);

	// The transformed version of the region of interest
	std::vector<std::complex<scalar_t> > features_;
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
	features_.resize(roi_size_2 * roi_size_2);

	// Fill the feature image
	for(unsigned int y = 0; y < roi_size; ++y) {
		for(unsigned int x = 0; x < roi_size; ++x) {
			features_[y * roi_size_2 + x].real() = pLines[y0 + y][x0 + x] / scalar_t(255);
		}
	}

	// Do 1D ffts along the rows before doing 1D ffts along the columns
	for(unsigned int y = 0; y < roi_size_2; ++y) {
		fft(&features_[y * roi_size_2], roi_size_2);
	}

	for(unsigned int x = 0; x < roi_size_2; ++x) {
		fft(&features_[x], roi_size_2, roi_size_2);
	}

	// Convert to magnitude/phase
	for(unsigned int i = 0; i < features_.size(); ++i) {
		features_[i] = std::complex<scalar_t>(std::abs(features_[i]), std::arg(features_[i]));
	}
}

scalar_t FourierHeuristic::computeFeature(unsigned int feature_index) {
	// Separate the real and imaginary part of the image (not necessary)
	if(feature_index < features_.size()) {
		return features_[feature_index].real();
	}
	else {
		return features_[feature_index - features_.size()].imag();
	}
}

// Round up to the next highest power of 2
unsigned int FourierHeuristic::roundUp(unsigned int x) {
	// Assume x is 32 bits
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return ++x;
}

// Fast fourier transform (adapted from the book Numerical Recipes, 3rd Ed.)
void FourierHeuristic::fft(std::complex<scalar_t>* data,
						   unsigned int n,
						   unsigned int stride,
						   bool inverse) {
	// Bit-reversal section
	for(unsigned int i = 0, j = 0; i < n; ++i) {
		if(i < j) { // Exchange the 2 complex number
			std::swap(data[i * stride], data[j * stride]);
		}

		unsigned int m = n / 2;

		while(m >= 1 && j >= m) {
			j -= m;
			m >>= 1;
		}

		j += m;
	}

	// Danielson-Lanczos section
	unsigned int mmax = 1;

	while(mmax < n) {
		const unsigned int istep = mmax << 1;
		scalar_t theta = inverse ? -M_PI / mmax : M_PI / mmax;
		std::complex<scalar_t> wp(std::cos(theta) - 1, std::sin(theta));
		std::complex<scalar_t> w(1, 0);

		for(unsigned int m = 0; m < mmax; ++m) {
			for(unsigned int i = m; i < n; i += istep) {
				unsigned int j = i + mmax;
				std::complex<scalar_t> temp = w * data[j * stride];
				data[j * stride] = data[i * stride] - temp;
				data[i * stride] += temp;
			}

			w += w * wp;
		}

		mmax = istep;
	}
}
