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

#include <algorithm>
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
	static const unsigned int NB_RECTS = 2000;

	// The number of histogram bins
	static const unsigned int NB_BINS  = 9;

	// Type of a rectangle
	struct Rect {
		// Coordinates of the region where to compute the histogram
		int xMin, yMin, xMax, yMax;

		// The bin the rectangle is looking at
		unsigned int bin;
	};

	// Convolve an image with two filters (respectively along rows and columns)
	static bool convolve(scalar_t* pixels, int width, int height,
						 const scalar_t* kernel1, const scalar_t* kernel2, int length);

	// The rectangles
	std::vector<Rect> rects_;

	// The 12 integral images used to calculate the HoG's
	std::vector<scalar_t> sats_[NB_BINS];

	// The width of the current image
	unsigned int width_;
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

	// Get the pixels values of the image
	byte_t** pixels = image->grayLines();

	// Normalize the image
	std::vector<scalar_t> image1(width * height);

	for(unsigned int y = 0; y < height; ++y) {
		for(unsigned int x = 0; x < width; ++x) {
			image1[y * width + x] = pixels[y][x];
		}
	}

	std::vector<scalar_t> image5(image1);

	scalar_t sigma = std::max(width, height) / 512.0f;
	int half = std::max(1, int(std::ceil(sigma * 3.0f)));

	std::vector<scalar_t> kernel(10 * half + 1);

	for (int x = -half; x <= half; ++x) {
		kernel[x + half] = std::exp(-x * x / (2.0f * sigma * sigma)) / (std::sqrt(2.0f * M_PI) * sigma);
	}

	convolve(&image1[0], width, height, &kernel[0], &kernel[0], 2 * half + 1);

	for (int x = -5 * half; x <= 5 * half; ++x) {
		kernel[x + 5 * half] = std::exp(-x * x / (50.0f * sigma * sigma)) / (std::sqrt(2.0f * M_PI) * 5.0f * sigma);
	}

	convolve(&image5[0], width, height, &kernel[0], &kernel[0], 10 * half + 1);

	for (unsigned int i = 0; i < image1.size(); ++i) {
		image1[i] -= image5[i];
	}

	image5.clear();

	// Initialize the integral images
	for(unsigned int b = 0; b < NB_BINS; ++b) {
		sats_[b].resize(width * height, 0);
	}

	// Compute the gradient at every pixel
	for(unsigned int y = 1; y < height - 1; ++y) {
		for(unsigned int x = 1; x < width - 1; ++x) {
			scalar_t dx = image1[y * width + x + 1] - image1[y * width + x - 1];
			scalar_t dy = image1[(y + 1) * width + x] - image1[(y - 1) * width + x];

			// Compute the magnitude and the orientation
			scalar_t magnitude = std::sqrt(dx * dx + dy * dy);

			if(magnitude > 0) {
				// In the range [-pi, pi]
				scalar_t theta = std::atan2(dy, dx);

				if (theta < 0) {
					theta += M_PI;
				}

				theta *= NB_BINS / M_PI;

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

	// Save the width of the current image
	width_ = width;
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

	// Integral image formula: sum = sat(A) + sat(C) - sat(B) - sat(D)
	return sats_[bin][yMin * width_ + xMin] +
		   sats_[bin][yMax * width_ + xMax] -
		   sats_[bin][yMin * width_ + xMax] -
		   sats_[bin][yMax * width_ + xMin];
}

bool hogHeuristic::convolve(scalar_t* pixels, int width, int height,
							const scalar_t* kernel1, const scalar_t* kernel2, int length) {
	if (width < length || height < length || length < 3 || !(length & 1)) {
		return false;
	}

	// Temporary buffer
	scalar_t* tmp = new scalar_t[std::max(width, height)];

	if (!tmp) {
		return false;
	}

	// Half kernel length
	const int half = length >> 1;

	// Convolve rows
	for (int y = 0; y < height; ++y) {
		std::copy(pixels + y * width, pixels + y * width + width, tmp);

		for (int x = 0; x < half; ++x) {
			register scalar_t sum = 0.0f;

			for (int k = -half; k <= half; ++k) {
				sum += kernel1[k + half] * tmp[std::max(0, x + k)];
			}

			pixels[y * width + x] = sum;
		}

		for (int x = half; x < width - half; ++x) {
			register scalar_t sum = 0.0f;

			for (int k = -half; k <= half; ++k) {
				sum += kernel1[k + half] * tmp[x + k];
			}

			pixels[y * width + x] = sum;
		}

		for (int x = width - half; x < width; ++x) {
			register scalar_t sum = 0.0f;

			for (int k = -half; k <= half; ++k) {
				sum += kernel1[k + half] * tmp[std::min(width - 1, x + k)];
			}

			pixels[y * width + x] = sum;
		}
	}

	// Convolve columns
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			tmp[y] = pixels[y * width + x];
		}

		for (int y = 0; y < half; ++y) {
			register scalar_t sum = 0.0f;

			for (int k = -half; k <= half; ++k) {
				sum += kernel2[k + half] * tmp[std::max(0, y + k)];
			}

			pixels[y * width + x] = sum;
		}

		for (int y = half; y < height - half; ++y) {
			register scalar_t sum = 0.0f;

			for (int k = -half; k <= half; ++k) {
				sum += kernel2[k + half] * tmp[y + k];
			}

			pixels[y * width + x] = sum;
		}

		for (int y = height - half; y < height; ++y) {
			register scalar_t sum = 0.0f;

			for (int k = -half; k <= half; ++k) {
				sum += kernel2[k + half] * tmp[std::min(height - 1, y + k)];
			}

			pixels[y * width + x] = sum;
		}
	}

	delete[] tmp;

	return true;
}
