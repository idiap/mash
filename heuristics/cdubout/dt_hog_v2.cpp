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
 *  Histogram of oriented gradients taken in a grid.
 *  Strongly inspired from leonidas/dt_hogs and francoisfleuret/zk_v2.
 */

#include <mash/heuristic.h>

#include <cmath>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'hog' heuristic class
//------------------------------------------------------------------------------
class hogHeuristic: public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual void init();
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual void finishForImage();
	virtual void prepareForCoordinates();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	static const unsigned int NB_BINS   = 9;
	static const unsigned int WIDTH     = 8;

	// The number of histograms
	unsigned int nbHists_;

	// The nbBins integral images used to calculate the HoG's
	std::vector<scalar_t> sats_[NB_BINS];

	// The vector used to store the histograms at a certain position
	std::vector<scalar_t> hists_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new hogHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

void hogHeuristic::init() {
	hists_.resize(dim());
}

unsigned int hogHeuristic::dim() {
	nbHists_ = ((roi_extent - WIDTH / 2) / WIDTH) * 2 + 1;
	return nbHists_ * nbHists_ * NB_BINS;
}

void hogHeuristic::prepareForImage() {
	// Get the size of the image (+1 as it is an integral image)
	const unsigned int width = image->width() + 1;
	const unsigned int height = image->height() + 1;

	// Initialize the integral images
	for(unsigned int b = 0; b < NB_BINS; ++b) {
		sats_[b].resize(width * height, 0);
	}

	// Get the pixels values of the image
	byte_t** pixels = image->grayLines();

	// Compute the gradient at every pixel
	for(unsigned int y = 1; y < height - 2; ++y) {
		for(unsigned int x = 1; x < width - 2; ++x) {
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

				// Convert theta to the range [0, 2 * NB_BINS]
				const scalar_t pi = 3.1415926536;

				theta = (theta + pi) * NB_BINS / pi;

				// Convert theta to the range [0, NB_BINS)
				while(theta >= NB_BINS) {
					theta -= NB_BINS;
				}

				if(theta < 0) {
					theta = 0;
				}

				// Bilinear interpolation
				unsigned int theta0 = (unsigned int)theta;
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
	for(unsigned int b = 0; b < NB_BINS; ++b) {
		sats_[b].clear();
	}
}

void hogHeuristic::prepareForCoordinates() {
	// Get the width of the image (+1 as it is an integral image)
	const unsigned int width = image->width() + 1;

	// Compute the extent of the histograms
	const unsigned int hist_extent = (WIDTH / 2) + (nbHists_ / 2) * WIDTH;

	for(unsigned int b = 0, index = 0; b < NB_BINS; ++b) {
		for(unsigned int i = 0, y = coordinates.y - hist_extent;
			i < nbHists_; ++i, y += WIDTH) {
			for(unsigned int j = 0, x = coordinates.x - hist_extent;
				j < nbHists_; ++j, x += WIDTH) {
				hists_[index] = sats_[b][y * width + x] +
								sats_[b][(y + WIDTH) * width + (x + WIDTH)] -
								sats_[b][(y + WIDTH) * width + x] -
								sats_[b][y * width + (x + WIDTH)];

				++index;
			}
		}
	}

	// Block normalization
	for(unsigned int i = 0; i < nbHists_; ++i) {
		for(unsigned int j = 0; j < nbHists_; ++j) {
			scalar_t norm = 0;

			for(unsigned int b = 0; b < NB_BINS; ++b) {
				norm += hists_[b * nbHists_ * nbHists_ + i * nbHists_ + j] *
						hists_[b * nbHists_ * nbHists_ + i * nbHists_ + j];
			}

			norm = 1 / std::sqrt(norm);

			for(unsigned int b = 0; b < NB_BINS; ++b) {
				hists_[b * nbHists_ * nbHists_ + i * nbHists_ + j] *= norm;

			//	if(hists_[b * nbHists_ * nbHists_ + i * nbHists_ + j] > 0.2) {
			//		hists_[b * nbHists_ * nbHists_ + i * nbHists_ + j] = 0.2;
			//	}
			}
		}
	}
}

scalar_t hogHeuristic::computeFeature(unsigned int feature_index) {
	return hists_[feature_index];
}
