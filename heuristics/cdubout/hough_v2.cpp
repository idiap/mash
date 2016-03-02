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
 *  Hough transform heuristic (weighted). Compute the linear hough transform of
 *  the region of interest to detect lines. Discretize the theta parameter in 36
 *  bins (from -90 to +85 degrees) and rho (the distance to the center, in the
 *  range +- sqrt(2) * roi_extent) in roi_size bins.
 */

#include <mash/heuristic.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'Hough' heuristic class
//------------------------------------------------------------------------------
class HoughHeuristic : public Heuristic {
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
extern "C" Heuristic* new_heuristic() {
	return new HoughHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

HoughHeuristic::HoughHeuristic() {
	for(int t = 0; t < 36; ++t) {
		// Convert to radian (from -90 to +85 degrees)
		scalar_t theta = (t - 18) * scalar_t(3.141592653589793) / 36;

		// Predivide by sqrt(2) so that we won't have to divide later
		sines_[t]   = std::sin(theta) / scalar_t(1.414213562);
		cosines_[t] = std::cos(theta) / scalar_t(1.414213562);
	}
}

unsigned int HoughHeuristic::dim() {
	// We have 36 * roi_size features
	int roi_size = roi_extent * 2 + 1;
	return 36 * roi_size;
}

void HoughHeuristic::prepareForCoordinates() {
	// Compute the coordinates of the top-left pixel of the region of interest
	int x0 = coordinates.x - roi_extent;
	int y0 = coordinates.y - roi_extent;

	// Compute the size of the region of interest
	int roi_size = roi_extent * 2 + 1;

	// Get the pixels values of the region of interest
	byte_t** pLines = image->grayLines();

	// Resize the features to the correct dimension
	features_.clear();
	features_.resize(dim());

	for(int y = 1; y < roi_size - 1; ++y) {
		for(int x = 1; x < roi_size - 1; ++x) {
			// Sobel filter
			int dx = -	   (int)pLines[y0 + y - 1][x0 + x - 1]
					 - 2 * (int)pLines[y0 + y    ][x0 + x - 1]
					 -	   (int)pLines[y0 + y + 1][x0 + x - 1]
					 +	   (int)pLines[y0 + y - 1][x0 + x + 1]
					 + 2 * (int)pLines[y0 + y    ][x0 + x + 1]
					 +	   (int)pLines[y0 + y + 1][x0 + x + 1];
			int dy = -	   (int)pLines[y0 + y - 1][x0 + x - 1]
					 - 2 * (int)pLines[y0 + y - 1][x0 + x    ]
					 -	   (int)pLines[y0 + y - 1][x0 + x + 1]
					 +	   (int)pLines[y0 + y + 1][x0 + x - 1]
					 + 2 * (int)pLines[y0 + y + 1][x0 + x    ]
					 +	   (int)pLines[y0 + y + 1][x0 + x + 1];

			// Compute the magnitude
			scalar_t magnitude = std::sqrt(scalar_t(dx * dx + dy * dy));

			// For every possible theta
			for(int t = 0; t < 36; ++t) {
				// Calculate rho (rho is in the range +- sqrt(2) * roi_extent)
				scalar_t rho = (x - (int)roi_extent) * cosines_[t] +
							   (y - (int)roi_extent) * sines_[t] +
							   roi_extent;

				// Increment the rho/theta pair in the feature matrix by the
				// magnitude of the edge

				// Bilinear interpolation
				int before = (int)rho;
				int after = before + 1;
				scalar_t alpha = rho - before;

				features_[before * 36 + t] += (1 - alpha) * magnitude;
				features_[after  * 36 + t] +=      alpha  * magnitude;
			}
		}
	}

	// Normalize the feature matrix
/*	const scalar_t max = *std::max_element(features_.begin(), features_.end());

	if(max > 0.0f) {
		const scalar_t invMax = scalar_t(1) / max;

		for(unsigned int i = 0; i < features_.size(); ++i) {
			features_[i] *= invMax;
		}
	}*/
}

scalar_t HoughHeuristic::computeFeature(unsigned int feature_index) {
	return features_[feature_index];
}
