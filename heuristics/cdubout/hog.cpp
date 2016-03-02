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
 *  Histogram of oriented gradients (orientation discretized in 12 bins) taken
 *  at random positions and scales.
 *  Strongly inspired from francoisfleuret/zk_v2.
 */

#include <mash/heuristic.h>

#include <cmath>
#include <cstdlib>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'HoG' heuristic class
//------------------------------------------------------------------------------
class HoGHeuristic: public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	HoGHeuristic();
	virtual unsigned int dim();
	virtual void prepareForCoordinates();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// The number of rectangles
	static const unsigned int nbRects = 1000;

	// Type of a rectangle
	struct Rect {
		// Coordinates of the region where to compute the histogram
		int xMin, yMin, xMax, yMax;

		// The bin the rectangle is looking at
		unsigned int bin;
	};

	// The rectangles
	std::vector<Rect> rects_;

	// The 12 integral images used to calculate the HoG's
	std::vector<scalar_t> sats_[12];
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new HoGHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

HoGHeuristic::HoGHeuristic() : rects_(nbRects) {
	// Nothing to do
}

unsigned int HoGHeuristic::dim() {
	// Compute the size of the region of interest
	const int roi_size = roi_extent * 2 + 1;

	// Initialize the 12 integral images
	for(unsigned int i = 0; i < 12; ++i) {
		sats_[i].resize(roi_size * roi_size);
	}

	// Randomize the nbRects rectangles (4 = minimum size of a rectangle)
	for(unsigned int r = 0; r < nbRects; ++r) {
		do {
			rects_[r].xMin = (std::rand() % roi_size) - (int)roi_extent;
			rects_[r].yMin = (std::rand() % roi_size) - (int)roi_extent;
			rects_[r].xMax = (std::rand() % roi_size) - (int)roi_extent;
			rects_[r].yMax = (std::rand() % roi_size) - (int)roi_extent;
		} while((rects_[r].xMax < rects_[r].xMin + 4) ||
				(rects_[r].yMax < rects_[r].yMin + 4));

		rects_[r].bin = std::rand() % 12;
	}

	return nbRects;
}

void HoGHeuristic::prepareForCoordinates() {
	// Compute the coordinates of the top-left pixel of the region of interest
	const unsigned int x0 = coordinates.x - roi_extent;
	const unsigned int y0 = coordinates.y - roi_extent;

	// Compute the size of the region of interest
	const unsigned int roi_size = roi_extent * 2 + 1;

	// Get the pixels values of the region of interest
	byte_t** pixels = image->grayLines();

	// Zero out the 12 integral images
	for(unsigned int i = 0; i < 12; ++i) {
		std::fill(sats_[i].begin(), sats_[i].end(), 0);
	}

	// Compute the gradient at every pixel
	for(unsigned int y = 1; y < roi_size - 1; ++y) {
		for(unsigned int x = 1; x < roi_size - 1; ++x) {
			// Sobel filter
			scalar_t dx = -     (int)pixels[y0 + y - 1][x0 + x - 1]
						  - 2 * (int)pixels[y0 + y    ][x0 + x - 1]
						  -     (int)pixels[y0 + y + 1][x0 + x - 1]
						  +     (int)pixels[y0 + y - 1][x0 + x + 1]
						  + 2 * (int)pixels[y0 + y    ][x0 + x + 1]
						  +     (int)pixels[y0 + y + 1][x0 + x + 1];
			scalar_t dy =       (int)pixels[y0 + y - 1][x0 + x - 1]
						  - 2 * (int)pixels[y0 + y - 1][x0 + x    ]
						  -     (int)pixels[y0 + y - 1][x0 + x + 1]
						  +     (int)pixels[y0 + y + 1][x0 + x - 1]
						  + 2 * (int)pixels[y0 + y + 1][x0 + x    ]
						  +     (int)pixels[y0 + y + 1][x0 + x + 1];

			// Compute the magnitude and the orientation
			scalar_t magnitude = std::sqrt(dx * dx + dy * dy);

			if(magnitude > 0) {
				// In the range [-pi, pi]
				scalar_t theta = std::atan2(dy, dx);

				// Convert theta to the range [0, 12]
				const scalar_t pi = 3.1415926536;

				theta = theta * 6 / pi + 6;

				// Just to make sure it's really [0, 12) and not [0, 12]
				if(theta >= 12) {
					theta -= 12;
				}

				// Bilinear interpolation
				int theta0 = (int)theta;
				int theta1 = theta0 + 1;
				scalar_t alpha = theta - theta0;

				if(theta1 == 12) {
					theta1 = 0;
				}

				sats_[theta0][y * roi_size + x] += (1 - alpha) * magnitude;
				sats_[theta1][y * roi_size + x] +=      alpha  * magnitude;
			}
		}
	}

	// Convert the gradient images to integral images
	// Integral image formula: sat(D) = i(D) + sat(B) + sat(D) - sat(A)
	for(unsigned int i = 0; i < 12; ++i) {
		for(unsigned int y = 1; y < roi_size; ++y) {
			for(unsigned int x = 1; x < roi_size; ++x) {
				sats_[i][y * roi_size + x] += sats_[i][y * roi_size + x - 1] +
											  sats_[i][(y - 1) * roi_size + x] -
											  sats_[i][(y - 1) * roi_size + x - 1];
			}
		}
	}
}

scalar_t HoGHeuristic::computeFeature(unsigned int feature_index) {
	// Compute the size of the region of interest
	const unsigned int roi_size = roi_extent * 2 + 1;
	const unsigned int xMin = coordinates.x + rects_[feature_index].xMin;
	const unsigned int yMin = coordinates.y + rects_[feature_index].yMin;
	const unsigned int xMax = coordinates.x + rects_[feature_index].xMax;
	const unsigned int yMax = coordinates.y + rects_[feature_index].yMax;
	const unsigned int bin = rects_[feature_index].bin;

	// Integral image formula: sum = sat(A) + sat(C) - sat(B) - sat(D)
	return sats_[bin][yMin * roi_size + xMin] +
		   sats_[bin][yMax * roi_size + xMax] -
		   sats_[bin][yMin * roi_size + xMax] -
		   sats_[bin][yMax * roi_size + xMin];
}
