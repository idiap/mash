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
 *  Segment the image into height regions, with equal number of pixels in each.
 */

#include <mash/heuristic.h>

#include <algorithm>
#include <numeric>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'segmentation8' heuristic class
//------------------------------------------------------------------------------
class Segmentation8 : public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// The splitting grayscale values
	unsigned char splits_[8];
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new Segmentation8();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int Segmentation8::dim() {
	// There is as many features than pixels in the region of interest
	unsigned int roi_size = roi_extent * 2 + 1;
	return roi_size * roi_size;
}

void Segmentation8::prepareForImage() {
	// Get the size of the image
	unsigned int width = image->width();
	unsigned int height = image->height();

	// Get the pixels of the region of interest
	byte_t** pLines = image->grayLines();

	// Histogram of the pixels
	unsigned int hist[256];
	std::fill(hist, hist + 256, 0U);

	for(unsigned int y = 0; y < height; ++y) {
		for(unsigned int x = 0; x < width; ++x) {
			++hist[pLines[y][x]];
		}
	}

	std::partial_sum(hist, hist + 256, hist);

	std::fill(splits_, splits_ + 8, 0U);

	for(unsigned int i = 0; i < 7; ++i) {
		while(splits_[i] < 255 && hist[splits_[i] + 1] <= width * height * (i + 1) / 8) {
			++splits_[i];
		}

		splits_[i + 1] = splits_[i];
	}

	splits_[7] = 255;
}

scalar_t Segmentation8::computeFeature(unsigned int feature_index) {
	// Compute the coordinates of the top-left pixel of the region of interest
	unsigned int x0 = coordinates.x - roi_extent;
	unsigned int y0 = coordinates.y - roi_extent;

	// Compute the coordinates of the pixel corresponding to the feature, in
	// the region of interest
	unsigned int roi_size = roi_extent * 2 + 1;
	unsigned int x = feature_index % roi_size;
	unsigned int y = (feature_index - x) / roi_size;

	// Return the pixel value corresponding to the desired feature
	byte_t** pLines = image->grayLines();

	unsigned int h = 0;

	while(pLines[y0 + y][x0 + x] > splits_[h]) {
		++h;
	}

	return h;
}
