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
 *  Strongly blurred gradient magnitude image.
 *  Inspired from francoisfleuret/chamferzk.
 */

#include <mash/heuristic.h>

#include <cmath>
#include <cstdlib>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'gradientblurred' heuristic class
//------------------------------------------------------------------------------
class GradientBlurred: public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual void finishForImage();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// Progressive blur method
	static void progressiveBlur(scalar_t* image, scalar_t* tmp, unsigned int width, unsigned int height, unsigned int k);

	// The blured gradient image
	std::vector<scalar_t> image_;

	// The width of the current image
	unsigned int width_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new GradientBlurred();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int GradientBlurred::dim() {
	// There is as many features than pixels in the region of interest
	unsigned int roi_size = roi_extent * 2 + 1;
	return roi_size * roi_size;
}

void GradientBlurred::prepareForImage() {
	// Get the size of the image
	unsigned int width = image->width();
	unsigned int height = image->height();

	// Initialize the gradient image
	image_.resize(width * height, 0);

	// Get the pixels values of the image
	byte_t** pixels = image->grayLines();

	// Compute the gradient at every pixel
	for(unsigned int y = 1; y < height - 1; ++y) {
		for(unsigned int x = 1; x < width - 1; ++x) {
			scalar_t dx = (int)pixels[y][x + 1] - (int)pixels[y][x - 1];
			scalar_t dy = (int)pixels[y + 1][x] - (int)pixels[y - 1][x];

			// Compute the magnitude of the gradient
			image_[y * width + x] = std::sqrt(dx * dx + dy * dy) / 2 / 255;
		}
	}

	// Approximate Gaussian blur with 3 progressive blurs
	std::vector<scalar_t> tmp(image_.size());
	progressiveBlur(&image_[0], &tmp[0], width, height, roi_extent / 3 + 1);
	progressiveBlur(&image_[0], &tmp[0], width, height, roi_extent / 3 + 1);
	progressiveBlur(&image_[0], &tmp[0], width, height, roi_extent / 3 + 1);

	// Save the width of the image
	width_ = width;
}

void GradientBlurred::finishForImage() {
	// Clear the image
	image_.clear();
}

scalar_t GradientBlurred::computeFeature(unsigned int feature_index) {
	// Compute the coordinates of the top-left pixel of the region of interest
	unsigned int x0 = coordinates.x - roi_extent;
	unsigned int y0 = coordinates.y - roi_extent;

	// Compute the coordinates of the pixel corresponding to the feature, in
	// the region of interest
	unsigned int roi_size = roi_extent * 2 + 1;
	unsigned int x = feature_index % roi_size;
	unsigned int y = (feature_index - x) / roi_size;

	return image_[(y0 + y) * width_ + x0 + x];
}

void GradientBlurred::progressiveBlur(scalar_t* image, scalar_t* tmp, unsigned int width, unsigned int height, unsigned int k) {
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
