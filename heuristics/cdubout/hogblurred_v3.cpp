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
 *  Strongly blurred hog images.
 *  Inspired from francoisfleuret/chamferzk.
 */

#include <mash/heuristic.h>

#include <cmath>
#include <cstdlib>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'hogblurred' heuristic class
//------------------------------------------------------------------------------
class HogBlurred : public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual void finishForImage();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// The number of histogram bins
	static const unsigned int NB_BINS = 8;

	// Progressive blur method
	static void progressiveBlur(scalar_t* image, scalar_t* tmp, unsigned int width, unsigned int height, unsigned int k);

	// The blurred images
	std::vector<scalar_t> images_[NB_BINS];

	// The width of the current image
	unsigned int width_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new HogBlurred();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int HogBlurred::dim() {
	return roi_extent * roi_extent * NB_BINS;
}

void HogBlurred::prepareForImage() {
	// Get the size of the image
	unsigned int width = image->width() / 2;
	unsigned int height = image->height() / 2;

	// Initialize the images
	for(unsigned int b = 0; b < NB_BINS; ++b) {
		images_[b].resize(width * height, 0);
	}

	// Get the pixels values of the image
	byte_t** pixels = image->grayLines();

	// Compute the gradient at every pixel
	for(unsigned int y = 1; y < height * 2 - 1; ++y) {
		for(unsigned int x = 1; x < width * 2 - 1; ++x) {
			scalar_t dx = (int)pixels[y][x + 1] - (int)pixels[y][x - 1];
			scalar_t dy = (int)pixels[y + 1][x] - (int)pixels[y - 1][x];

			// Compute the magnitude of the gradient
			scalar_t magnitude = std::sqrt(dx * dx + dy * dy) / 2 / 255;

			if(magnitude > 0) {
				// In the range [-pi, pi]
				scalar_t theta = std::atan2(dy, dx);

				// Convert theta to the range [0, NB_BINS]
				const scalar_t pi = 3.1415926536;

				theta = (theta + pi) * NB_BINS / (2 * pi);

				// Just to make sure it's really [0, NB_BINS)
				if(theta >= NB_BINS) {
					theta = 0;
				}

				// Bilinear interpolation
				unsigned int theta0 = (int)theta;
				unsigned int theta1 = theta0 + 1;
				scalar_t alpha = theta - theta0;

				if(theta1 == NB_BINS) {
					theta1 = 0;
				}

				images_[theta0][(y / 2) * width + x / 2] += (1 - alpha) * magnitude;
				images_[theta1][(y / 2) * width + x / 2] +=      alpha  * magnitude;
			}
		}
	}

	// Approximate Gaussian blur with 3 progressive blurs
	std::vector<scalar_t> tmp(images_[0].size());

	for(unsigned int b = 0; b < NB_BINS; ++b) {
		progressiveBlur(&images_[b][0], &tmp[0], width, height, roi_extent / 20 + 1);
		progressiveBlur(&images_[b][0], &tmp[0], width, height, roi_extent / 20 + 1);
		progressiveBlur(&images_[b][0], &tmp[0], width, height, roi_extent / 20 + 1);
	}

	// Save the width of the image
	width_ = width;
}

void HogBlurred::finishForImage() {
	// Clear the images
	for(unsigned int b = 0; b < NB_BINS; ++b) {
		images_[b].clear();
	}
}

scalar_t HogBlurred::computeFeature(unsigned int feature_index) {
	// Compute the coordinates of the top-left pixel of the region of interest
	unsigned int x0 = (coordinates.x - roi_extent) / 2;
	unsigned int y0 = (coordinates.y - roi_extent) / 2;

	// Compute the coordinates of the pixel corresponding to the feature, in
	// the region of interest
	unsigned int y = feature_index / (roi_extent * NB_BINS);
	feature_index -= y * roi_extent * NB_BINS;
	unsigned int b = feature_index / roi_extent;
	unsigned int x = feature_index % roi_extent;

	return images_[b][(y0 + y) * width_ + x0 + x];
}

void HogBlurred::progressiveBlur(scalar_t* image, scalar_t* tmp, unsigned int width, unsigned int height, unsigned int k) {
	const long double norm = 1.0 / (2 * k + 1);

	for(unsigned int y = 0; y < height; ++y) {
		long double sum = 0;

		for(unsigned int x = 0; x <= k; ++x) {
			sum = 0;

			for(int i = int(x) - int(k); i <= int(x + k); ++i) {
				sum += image[y * width + std::abs(i)];
			}

			tmp[y * width + x] = sum * norm;
		}

		for(unsigned int x = k + 1; x < width - k; ++x) {
			sum += image[y * width + x + k] - image[y * width + x - k - 1];
			tmp[y * width + x] = sum * norm;
		}

		for(unsigned int x = width - k; x < width; ++x) {
			sum = 0;

			for(unsigned int i = x - k; i <= x + k; ++i) {
				sum += image[y * width + std::min(i, 2 * width - 2 - i)];
			}

			tmp[y * width + x] = sum * norm;
		}
	}

	for(unsigned int x = 0; x < width; ++x) {
		long double sum = 0;

		for(unsigned int y = 0; y <= k; ++y) {
			sum = 0;

			for(int i = int(y) - int(k); i <= int(y + k); ++i) {
				sum += tmp[std::abs(i) * width + x];
			}

			image[y * width + x] = sum * norm;
		}

		for(unsigned int y = k + 1; y < height - k; ++y) {
			sum += tmp[(y + k) * width + x] - tmp[(y - k - 1) * width + x];
			image[y * width + x] = sum * norm;
		}

		for(unsigned int y = height - k; y < height; ++y) {
			sum = 0;

			for(unsigned int i = y - k; i <= y + k; ++i) {
				sum += tmp[std::min(i, 2 * height - 2 - i) * width + x];
			}

			image[y * width + x] = sum * norm;
		}
	}
}
