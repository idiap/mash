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

    Hough transform heuristic. Compute the linear hough transform of the image
	to detect lines. Discretize the theta parameter in 36 bins (from -90 to 85
    degrees) and rho (the distance to the center) in roi_extent + 1 bins.
*/

#include <mash/heuristic.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'Hough' heuristic class
//------------------------------------------------------------------------------
class HoughHeuristic : public Heuristic
{
    //_____ Implementation of Heuristic __________
public:
	HoughHeuristic();
    virtual unsigned int dim();
	virtual void prepareForCoordinates();
    virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// The hough matrix of features
	std::vector<scalar_t> features_;

	// Lookup tables of sines and cosines
	scalar_t sines_[36];
	scalar_t cosines_[36];
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new HoughHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

HoughHeuristic::HoughHeuristic() {
	for(int t = 0; t < 36; ++t) {
		// Convert to radian
		scalar_t theta = (t - 18) * scalar_t(3.141592653589793) / 36;

		// Predivide by 2 * sqrt(2) so that we won't have to divide later
		sines_[t]   = std::sin(theta) / (2 * scalar_t(1.414213562));
		cosines_[t] = std::cos(theta) / (2 * scalar_t(1.414213562));
	}
}

unsigned int HoughHeuristic::dim() {
	// We have 36 * roi_extent + 1 features
	return 36 * (roi_extent + 1);
}

void HoughHeuristic::prepareForCoordinates() {
	// Compute the coordinates of the top-left pixel of the region of interest
	int x0 = coordinates.x - roi_extent;
	int y0 = coordinates.y - roi_extent;

	// Compute the size of the region of interest
	int roi_size = roi_extent * 2 + 1;

	// Get the pixels values of the region of interest
	byte_t** pLines = image->grayLines();

	// Compute the edge image using a sobel filter
	std::vector<scalar_t> edge_image(roi_size * roi_size, scalar_t());

	for(int y = 1; y < roi_size - 1; ++y) {
		for(int x = 1; x < roi_size - 1; ++x) {
			// Sobel filter
			int dx = -	   (int)pLines[y0 + y - 1][x0 + x - 1]
					 - 2 * (int)pLines[y0 + y    ][x0 + x - 1]
					 -	   (int)pLines[y0 + y + 1][x0 + x - 1]
					 +	   (int)pLines[y0 + y - 1][x0 + x + 1]
					 + 2 * (int)pLines[y0 + y    ][x0 + x + 1]
					 +	   (int)pLines[y0 + y - 1][x0 + x + 1];
			int dy = -	   (int)pLines[y0 + y - 1][x0 + x - 1]
					 - 2 * (int)pLines[y0 + y - 1][x0 + x    ]
					 -	   (int)pLines[y0 + y - 1][x0 + x + 1]
					 +	   (int)pLines[y0 + y + 1][x0 + x - 1]
					 + 2 * (int)pLines[y0 + y    ][x0 + x    ]
					 +	   (int)pLines[y0 + y + 1][x0 + x + 1];
			// Store the magnitude
			edge_image[y * roi_size + x] =
				std::sqrt(scalar_t(dx * dx + dy * dy));
		}
	}

	// Resize the features to the correct dimension
	features_.clear();
	features_.resize(dim(), scalar_t());

	for(int y = 1; y < roi_size - 1; ++y) {
		for(int x = 1; x < roi_size - 1; ++x) {
			// For every possible theta
			for(int t = 0; t < 36; ++t) {
				// Calculate rho (rho is in between -sqrt(2) * roi_extent and
				// sqrt(2) * roi_extent)
				scalar_t rho = (x - (int)roi_extent) * cosines_[t] +
							   (y - (int)roi_extent) * sines_[t] +
							   roi_extent * scalar_t(0.5);

				// Increment the corresponding rho/theta pair in the feature
				// matrix by the corresponding magnitude of the edge

				// Bilinear interpolation
				int before = (int)rho;
				int after = before + 1;
				scalar_t alpha = rho - before;

				features_[before * 36 + t] +=
					(1 - alpha) * edge_image[y * roi_size + x];

				features_[after * 36 + t] +=
					alpha * edge_image[y * roi_size + x];
			}
		}
	}
}

scalar_t HoughHeuristic::computeFeature(unsigned int feature_index) {
	return features_[feature_index];
}
