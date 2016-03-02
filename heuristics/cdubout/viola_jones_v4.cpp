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
 *  The rectangle features used by Viola and Jones in their famous IJCV paper
 *  "Robust Real-time Object Detection".
 */

#include <mash/heuristic.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Mash;
using namespace std;

//------------------------------------------------------------------------------
/// Class used to store an integral image
//------------------------------------------------------------------------------
class MyIntegralImage {
public:
	// Initialize the integral image from a pixel array
	double init(byte_t** pixels,
				unsigned int width,
				unsigned int height);

	// Returns the sum of the pixel intensities in a given rectangle
	int operator()(unsigned int x,
				   unsigned int y,
				   unsigned int w,
				   unsigned int h) const;

private:
	vector<int> image_;
	unsigned int stride_;
};

//------------------------------------------------------------------------------
/// The 'ViolaJones' heuristic class
//------------------------------------------------------------------------------
class ViolaJones : public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	virtual void init();
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// Type of a rectangle feature
	struct Rectangle {
		unsigned int x0, y0, w0, h0;
		unsigned int x1, y1, w1, h1;
		unsigned int x2, y2, w2, h2;
		int a0, a1, a2;
	};

	// All the possible features
	vector<Rectangle> rectangles_;

	// The integral image of the current image
	MyIntegralImage integralImage_;

	// The standard deviation of the current image
	double stdDev_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new ViolaJones();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

double MyIntegralImage::init(byte_t** pixels, unsigned int width,
						     unsigned int height) {
	// Resize the integral image
	image_.resize((width + 1) * (height + 1));
	stride_ = width + 1;

	// Recopy the pixel array in the integral image
	for (unsigned int x = 0; x <= width; ++x)
		image_[x] = 0;

	for (unsigned int y = 0; y <= height; ++y)
		image_[y * stride_] = 0;

	double sumSq = 0.0;

	for (unsigned int y = 1; y <= height; ++y)
		for (unsigned int x = 1; x <= width; ++x) {
			image_[y * stride_ + x] = pixels[y - 1][x - 1];
			sumSq += double(pixels[y - 1][x - 1]) * double(pixels[y - 1][x - 1]);
		}

	// Integral image formula
	for (unsigned int y = 1; y <= height; ++y)
		for (unsigned int x = 1; x <= width; ++x)
			image_[y * stride_ + x] += image_[y * stride_ + x - 1] +
									   image_[(y - 1) * stride_ + x] -
									   image_[(y - 1) * stride_ + x - 1];

	const double mean = double(image_[height * stride_ + width]) / (width * height);

	return std::sqrt(sumSq / (width * height) - mean * mean);
}

int MyIntegralImage::operator()(unsigned int x, unsigned int y, unsigned int w,
							    unsigned int h) const {
	return image_[(y + h) * stride_ + x + w] + image_[y * stride_ + x] -
		   image_[(y + h) * stride_ + x] - image_[y * stride_ + x + w];
}

void ViolaJones::init() {
	// Compute the size of the region of interest
	const unsigned int roi_size = 2 * roi_extent + 1;

	// In case the ROI is too big, rescale all the features
	const unsigned int scale = max(roi_size / 16, 1U);

	// Two vertical rectangles
	Rectangle rectangle;
	rectangle.a0 = 1;
	rectangle.a1 =-2;
	rectangle.a2 = 0;

	for (unsigned int h = scale; h <= roi_size; h += scale) {
		for (unsigned int w = 2 * scale; w <= roi_size; w += 2 * scale) {
			for (unsigned int y = 0; y <= roi_size - h; y += scale) {
				for (unsigned int x = 0; x <= roi_size - w; x += scale) {
					rectangle.x0 = x;
					rectangle.y0 = y;
					rectangle.w0 = w;
					rectangle.h0 = h;
					rectangle.x1 = x + w / 2;
					rectangle.y1 = y;
					rectangle.w1 = w / 2;
					rectangle.h1 = h;
					rectangles_.push_back(rectangle);
				}
			}
		}
	}

	// Two horizontal rectangles
	rectangle.a0 = 1;
	rectangle.a1 =-2;
	rectangle.a2 = 0;

	for (unsigned int h = 2 * scale; h <= roi_size; h += 2 * scale) {
		for (unsigned int w = scale; w <= roi_size; w += scale) {
			for (unsigned int y = 0; y <= roi_size - h; y += scale) {
				for (unsigned int x = 0; x <= roi_size - w; x += scale) {
					rectangle.x0 = x;
					rectangle.y0 = y;
					rectangle.w0 = w;
					rectangle.h0 = h;
					rectangle.x1 = x;
					rectangle.y1 = y + h / 2;
					rectangle.w1 = w;
					rectangle.h1 = h / 2;
					rectangles_.push_back(rectangle);
				}
			}
		}
	}

	// Three vertical rectangles
	rectangle.a0 = 1;
	rectangle.a1 =-3;
	rectangle.a2 = 0;

	for (unsigned int h = scale; h <= roi_size; h += scale) {
		for (unsigned int w = 3 * scale; w <= roi_size; w += 3 * scale) {
			for (unsigned int y = 0; y <= roi_size - h; y += scale) {
				for (unsigned int x = 0; x <= roi_size - w; x += scale) {
					rectangle.x0 = x;
					rectangle.y0 = y;
					rectangle.w0 = w;
					rectangle.h0 = h;
					rectangle.x1 = x + w / 3;
					rectangle.y1 = y;
					rectangle.w1 = w / 3;
					rectangle.h1 = h;
					rectangles_.push_back(rectangle);
				}
			}
		}
	}

	// Three horizontal rectangles
	rectangle.a0 = 1;
	rectangle.a1 =-3;
	rectangle.a2 = 0;

	for (unsigned int h = 3 * scale; h <= roi_size; h += 3 * scale) {
		for (unsigned int w = scale; w <= roi_size; w += scale) {
			for (unsigned int y = 0; y <= roi_size - h; y += scale) {
				for (unsigned int x = 0; x <= roi_size - w; x += scale) {
					rectangle.x0 = x;
					rectangle.y0 = y;
					rectangle.w0 = w;
					rectangle.h0 = h;
					rectangle.x1 = x;
					rectangle.y1 = y + h / 3;
					rectangle.w1 = w;
					rectangle.h1 = h / 3;
					rectangles_.push_back(rectangle);
				}
			}
		}
	}

	// Four diagonal rectangles
	rectangle.a0 = 1;
	rectangle.a1 =-2;
	rectangle.a2 =-2;

	for (unsigned int h = 2 * scale; h <= roi_size; h += 2 * scale) {
		for (unsigned int w = 2 * scale; w <= roi_size; w += 2 * scale) {
			for (unsigned int y = 0; y <= roi_size - h; y += scale) {
				for (unsigned int x = 0; x <= roi_size - w; x += scale) {
					rectangle.x0 = x;
					rectangle.y0 = y;
					rectangle.w0 = w;
					rectangle.h0 = h;
					rectangle.x1 = x;
					rectangle.y1 = y;
					rectangle.w1 = w / 2;
					rectangle.h1 = h / 2;
					rectangle.x2 = x + w / 2;
					rectangle.y2 = y + w / 2;
					rectangle.w2 = w / 2;
					rectangle.h2 = h / 2;
					rectangles_.push_back(rectangle);
				}
			}
		}
	}
}

unsigned int ViolaJones::dim() {
	return rectangles_.size();
}

void ViolaJones::prepareForImage() {
	stdDev_ = integralImage_.init(image->grayLines(), image->width(), image->height());
}

scalar_t ViolaJones::computeFeature(unsigned int feature_index) {
	if (stdDev_ > 0.0) {
		const unsigned int x = coordinates.x - roi_extent;
		const unsigned int y = coordinates.y - roi_extent;

		const Rectangle& r = rectangles_[feature_index];

		int sum = integralImage_(x + r.x0, y + r.y0, r.w0, r.h0) * r.a0 +
				  integralImage_(x + r.x1, y + r.y1, r.w1, r.h1) * r.a1;

		if (r.a2)
			sum += integralImage_(x + r.x2, y + r.y2, r.w2, r.h2) * r.a2;

		return sum / stdDev_;
	}
	else {
		return 0;
	}
}
