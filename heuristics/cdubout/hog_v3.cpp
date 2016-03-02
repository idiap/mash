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
 *  Histogram of oriented gradients taken at random positions and scales.
 *  Strongly inspired from francoisfleuret/zk_v2.
 */

#include <mash/heuristic.h>

#include <cmath>
#include <cstdlib>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'hog' heuristic class
//------------------------------------------------------------------------------
class hogHeuristic: public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	hogHeuristic();
	virtual void init();
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual void finishForImage();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// The number of rectangles
	static const unsigned int NB_RECTS = 1000;

	// The number of histogram bins
	static const unsigned int NB_BINS  = 12;

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
	std::vector<scalar_t> sats_[NB_BINS];
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new hogHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

hogHeuristic::hogHeuristic() : rects_(NB_RECTS) {
	// Nothing to do
}

void hogHeuristic::init() {
	// Compute the size of the region of interest
	const int roi_size = roi_extent * 2 + 1;

	// Randomize the nbRects rectangles (4 = minimum size of a rectangle)
	for(unsigned int r = 0; r < NB_RECTS; ++r) {
		do {
			rects_[r].xMin = (std::rand() % roi_size) - (int)roi_extent;
			rects_[r].yMin = (std::rand() % roi_size) - (int)roi_extent;
			rects_[r].xMax = (std::rand() % roi_size) - (int)roi_extent;
			rects_[r].yMax = (std::rand() % roi_size) - (int)roi_extent;
		} while((rects_[r].xMax <= rects_[r].xMin) ||
				(rects_[r].yMax <= rects_[r].yMin));

		rects_[r].bin = std::rand() % NB_BINS;
	}
}

unsigned int hogHeuristic::dim() {
	return NB_RECTS;
}

void hogHeuristic::prepareForImage() {
	// Get the size of the image
	const unsigned int width = image->width();
	const unsigned int height = image->height();

	// Initialize the integral images
	for(unsigned int b = 0; b < NB_BINS; ++b) {
		sats_[b].resize(width * height, 0);
	}

	// Get the pixels values of the image
	byte_t** pixels = image->grayLines();

	// Compute the gradient at every pixel
	for(unsigned int y = 1; y < height - 1; ++y) {
		for(unsigned int x = 1; x < width - 1; ++x) {
			// Sobel filter
			scalar_t dx = -     (int)pixels[y - 1][x - 1]
						  - 2 * (int)pixels[y    ][x - 1]
						  -     (int)pixels[y + 1][x - 1]
						  +     (int)pixels[y - 1][x + 1]
						  + 2 * (int)pixels[y    ][x + 1]
						  +     (int)pixels[y + 1][x + 1];
			scalar_t dy = -     (int)pixels[y - 1][x - 1]
						  - 2 * (int)pixels[y - 1][x    ]
						  -     (int)pixels[y - 1][x + 1]
						  +     (int)pixels[y + 1][x - 1]
						  + 2 * (int)pixels[y + 1][x    ]
						  +     (int)pixels[y + 1][x + 1];

			// Compute the magnitude and the orientation
			scalar_t magnitude = std::sqrt(dx * dx + dy * dy) / 255;

			if(magnitude > 0) {
				// In the range [-pi, pi]
				scalar_t theta = std::atan2(dy, dx);

				// Convert theta to the range [0, 12]
				const scalar_t pi = 3.1415926536;

				theta = (theta + pi) * NB_BINS / (2 * pi);

				// Just to make sure it's really [0, NB_BINS)
				if(theta >= NB_BINS) {
					theta -= NB_BINS;
				}

				if(theta < 0) {
					theta = 0;
				}

				// Bilinear interpolation
				unsigned int theta0 = (int)theta;
				unsigned int theta1 = theta0 + 1;
				scalar_t alpha = theta - theta0;

				if(theta1 == NB_BINS) {
					theta1 = 0;
				}

				sats_[theta0][y * width + x] += (1 - alpha) * magnitude;
				sats_[theta1][y * width + x] +=      alpha  * magnitude;
			}
		}
	}

	// Convert the gradient images to integral images
	// Integral image formula: sat(D) = i(D) + sat(B) + sat(D) - sat(A)
	for(unsigned int b = 0; b < NB_BINS; ++b) {
		for(unsigned int y = 1; y < height; ++y) {
			for(unsigned int x = 1; x < width; ++x) {
				sats_[b][y * width + x] += sats_[b][y * width + x - 1] +
										   sats_[b][(y - 1) * width + x] -
										   sats_[b][(y - 1) * width + x - 1];
			}
		}
	}
}

void hogHeuristic::finishForImage() {
	// Clear the integral images
	for(unsigned int i = 0; i < NB_BINS; ++i) {
		sats_[i].clear();
	}
}

scalar_t hogHeuristic::computeFeature(unsigned int feature_index) {
	const int xMin = (int)coordinates.x + rects_[feature_index].xMin;
	const int yMin = (int)coordinates.y + rects_[feature_index].yMin;
	const int xMax = (int)coordinates.x + rects_[feature_index].xMax;
	const int yMax = (int)coordinates.y + rects_[feature_index].yMax;
	const unsigned int bin = rects_[feature_index].bin;

	// Get the width of the image
	const unsigned int width = image->width();

	// Integral image formula: sum = sat(A) + sat(C) - sat(B) - sat(D)
	return sats_[bin][yMin * width + xMin] +
		   sats_[bin][yMax * width + xMax] -
		   sats_[bin][yMin * width + xMax] -
		   sats_[bin][yMax * width + xMin];
}
