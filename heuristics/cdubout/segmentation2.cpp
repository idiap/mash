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
 *  Segment the image into two regions, above and below the image average
 *  grayscale value.
 */

#include <mash/heuristic.h>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'segmentation2' heuristic class
//------------------------------------------------------------------------------
class Segmentation2: public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// The average grayscale value
	unsigned char avg_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new Segmentation2();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int Segmentation2::dim() {
	// There is as many features than pixels in the region of interest
	unsigned int roi_size = roi_extent * 2 + 1;
	return roi_size * roi_size;
}

void Segmentation2::prepareForImage() {
	// Get the size of the image
	unsigned int width = image->width();
	unsigned int height = image->height();

	// Get the pixels of the region of interest
	byte_t** pLines = image->grayLines();

	// Sum of the pixels
	unsigned int sum = 0;

	for(unsigned int y = 0; y < height; ++y) {
		for(unsigned int x = 0; x < width; ++x) {
			sum += pLines[y][x];
		}
	}

	avg_ = sum / (width * height);
}

scalar_t Segmentation2::computeFeature(unsigned int feature_index) {
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
	return pLines[y0 + y][x0 + x] > avg_;
}
