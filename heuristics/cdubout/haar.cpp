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
 *
 *  Haar transform heuristic. Compute the 2D haar transform of the region of
 *  interest (all levels), padding the ROI with zeros so that the size is a
 *  power of two.
 */

#include <mash/heuristic.h>

#include <cmath>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'Haar' heuristic class
//------------------------------------------------------------------------------
class HaarHeuristic: public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();
	virtual void prepareForCoordinates();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// Compute the 1D haar transform
	static void haar1(scalar_t* features, unsigned int w);

	// Compute the 2D haar transform
	static void haar2(scalar_t* features, unsigned int w);

	// Round up to the next highest power of 2
	static unsigned int roundUp(unsigned int x);

	// The transformed version of the region of interest
	std::vector<scalar_t> features_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new HaarHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int HaarHeuristic::dim() {
	// We have has many features than pixels in the region of interest (rounded
	// to the next highest power of 2)
	unsigned int roi_size = roi_extent * 2 + 1;
	unsigned int extended_size = roundUp(roi_size);
	return extended_size * extended_size;
}

void HaarHeuristic::prepareForCoordinates() {
	// Compute the coordinates of the top-left pixel of the region of interest
	unsigned int x0 = coordinates.x - roi_extent;
	unsigned int y0 = coordinates.y - roi_extent;

	// Compute the size of the region of interest
	unsigned int roi_size = roi_extent * 2 + 1;
	unsigned int extended_size = roundUp(roi_size);

	// Get the pixels values of the region of interest
	byte_t** pLines = image->grayLines();

	// Resize the features to the correct dimension
	features_.clear();
	features_.resize(dim());

	// Fill the feature image
	for(unsigned int y = 0; y < roi_size; ++y) {
		for(unsigned int x = 0; x < roi_size; ++x) {
			features_[y * extended_size + x] = pLines[y][x];
		}
	}

	haar2(&features_[0], extended_size);
}

scalar_t HaarHeuristic::computeFeature(unsigned int feature_index) {
	return features_[feature_index];
}

// Compute the 1D haar transform
void HaarHeuristic::haar1(scalar_t* features, unsigned int w) {
	const scalar_t invSqrt2 = 0.707106781187;

	std::vector<scalar_t> temp(w);

	for(unsigned int i = 0; i < (w / 2); ++i) {
		temp[i] = (features[2 * i] + features[2 * i + 1]) * invSqrt2;
		temp[i + (w / 2)] = (features[2 * i] - features[2 * i + 1]) * invSqrt2;
	}

	for(unsigned int i = 0; i < w; ++i) {
		features[i] = temp[i];
	}
}

// Compute the 2D haar transform
void HaarHeuristic::haar2(scalar_t* features, unsigned int w) {
	std::vector<scalar_t> temp(w);

	unsigned int z = w;

	while(z > 1) {
		// Do 1D haar transforms along the rows
		for(unsigned int i = 0; i < z; ++i) {
			haar1(features + i * w, z);
		}

		// Do 1D haar transforms along the columns
		for(unsigned int i = 0; i < z; ++i) {
			for(unsigned int j = 0; j < w; ++j) {
				temp[j] = features[i + j * w];
			}

			haar1(&temp[0], z);

			for(unsigned int j = 0; j < w; ++j) {
				features[i + j * w] = temp[j];
			}
		}

		z >>= 1;
	}
}

// Round up to the next highest power of 2
unsigned int HaarHeuristic::roundUp(unsigned int x) {
	// Assume x is 32-bits
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return ++x;
}
