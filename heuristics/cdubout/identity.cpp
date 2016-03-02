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
 *  Simple example of a heuristic: Identity. The features are a copy of the
 *  (rgb) pixels contained in the processed region of interest.
 */

#include <mash/heuristic.h>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'Identity' heuristic class
//------------------------------------------------------------------------------
class IdentityHeuristic: public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();

	virtual scalar_t computeFeature(unsigned int feature_index);
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new IdentityHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int IdentityHeuristic::dim() {
	// We have has many features than pixels in the region of interest times 3
	unsigned int roi_size = roi_extent * 2 + 1;
	return roi_size * roi_size * 3;
}

scalar_t IdentityHeuristic::computeFeature(unsigned int feature_index) {
	// Compute the coordinates of the top-left pixel of the region of interest
	unsigned int x0 = coordinates.x - roi_extent;
	unsigned int y0 = coordinates.y - roi_extent;

	// Compute the coordinates of the pixel corresponding to the feature, in
	// the region of interest
	unsigned int roi_size = roi_extent * 2 + 1;
	unsigned int c = feature_index % 3;
	unsigned int x = (feature_index / 3) % roi_size;
	unsigned int y = (feature_index / 3) / roi_size;

	// Return the pixel value corresponding to the desired feature
	RGBPixel_t** pLines = image->rgbLines();

	if(c == 0) {
		return (scalar_t) pLines[y0 + y][x0 + x].r;
	}
	else if(c == 1) {
		return (scalar_t) pLines[y0 + y][x0 + x].g;
	}
	else {
		return (scalar_t) pLines[y0 + y][x0 + x].b;
	}
}
