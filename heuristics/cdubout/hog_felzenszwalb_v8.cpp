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
 *  Histogram of oriented gradients, as described in the paper 'Object Detection
 *  with Discriminatively Trained Part Based Models' by Felzenszwalb et al.
 *  Strongly inspired from leonidas/rgb_hogs.
 */

#include <mash/heuristic.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Mash;
using namespace std;

//------------------------------------------------------------------------------
/// The 'HoG' heuristic class
//------------------------------------------------------------------------------
class HoGHeuristic: public Heuristic
{
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	inline void interpolate(int x, int y, int b, float magnitude);

	// The features of the current image
	vector<float> features_;

	// The width and height of the current feature map
	int width_;
	int height_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
	return new HoGHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int HoGHeuristic::dim()
{
	// Compute the size of the region of interest
	const unsigned int roi_size = roi_extent * 2 + 1;
	return ((roi_size + 7) >> 3) * ((roi_size + 7) >> 3) * 32;
}

void HoGHeuristic::prepareForImage()
{
	// Get the dimensions of the image
	const int width = image->width();
	const int height = image->height();

	// Get the pixels values of the image
	RGBPixel_t **pixels = image->rgbLines();

	// Set the dimensions of the current feature map
	width_ = (width + 7) >> 3;
	height_ = (height + 7) >> 3;

	// Resize the features
	features_.resize(width_ * height_ * 32);
	fill(features_.begin(), features_.end(), 0.0f);

	// Compute the gradient at every pixel
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			// Use the channel with the largest gradient magnitude
			int rDx = int(pixels[y][min(x + 1, width - 1)].r)
					- int(pixels[y][max(x - 1, 0)].r);
			int rDy = int(pixels[min(y + 1, height - 1)][x].r)
					- int(pixels[max(y - 1, 0)][x].r);

			int gDx = int(pixels[y][min(x + 1, width - 1)].g)
					- int(pixels[y][max(x - 1, 0)].g);
			int gDy = int(pixels[min(y + 1, height - 1)][x].g)
					- int(pixels[max(y - 1, 0)][x].g);

			int bDx = int(pixels[y][min(x + 1, width - 1)].b)
					- int(pixels[y][max(x - 1, 0)].b);
			int bDy = int(pixels[min(y + 1, height - 1)][x].b)
					- int(pixels[max(y - 1, 0)][x].b);

			const int rMagnitude = rDx * rDx + rDy * rDy;
			const int gMagnitude = gDx * gDx + gDy * gDy;
			const int bMagnitude = bDx * bDx + bDy * bDy;

			float magnitude = 0.0f;
			float theta = 0.0f;

			if (rMagnitude > gMagnitude && rMagnitude > bMagnitude) {
				magnitude = sqrtf(rMagnitude);
				theta = atan2f(rDy, rDx);
			}
			else if (gMagnitude > bMagnitude) {
				magnitude = sqrtf(gMagnitude);
				theta = atan2f(gDy, gDx);
			}
			else if (bMagnitude) {
				magnitude = sqrtf(bMagnitude);
				theta = atan2f(bDy, bDx);
			}

			if (magnitude > 0.0f) {
				// Convert theta to the range [0, 18)
				theta = max(theta * 9.0f / float(M_PI) + 9.5f, 0.0f);

				if (theta >= 18.0f)
					theta -= 18.0f;

				// Discretize theta
				const int theta0 = theta;
				const int theta1 = (theta0 < 17) ? (theta0 + 1) : 0;
				const float alpha = theta - theta0;

				interpolate(x, y, theta0 + 9, magnitude * (1.0f - alpha));
				interpolate(x, y, theta1 + 9, magnitude * alpha);

				// Unoriented gradient
				theta += 0.5f;

				if (theta >= 9.0f)
					theta -= 9.0f;

				// Discretize theta
				const int theta2 = theta;
				const int theta3 = (theta2 < 8) ? (theta2 + 1) : 0;
				const float beta = theta - theta2;

				interpolate(x, y, theta2, magnitude * (1.0f - beta));
				interpolate(x, y, theta3, magnitude * beta);
			}
		}
	}

	// Compute the "gradient energy" of each cell, i.e. ||C(i,j)||^2
	for (int i = 0; i < height_; ++i) {
		for (int j = 0; j < width_; ++j) {
			float sumSquare = 0.0f;

			for (int k = 0; k < 9; ++k)
				sumSquare += features_[(i * width_ + j) * 32 + k] * features_[(i * width_ + j) * 32 + k];

			features_[(i * width_ + j) * 32 + 31] = sumSquare;
		}
	}

	// Compute the four normalization factors and normalize and clamp everything
	for (int i = 0; i < height_; ++i) {
		for (int j = 0; j < width_; ++j) {
			float sum00 = features_[(max(i - 1, 0) * width_ + max(j - 1, 0)) * 32 + 31];
			float sum01 = features_[(max(i - 1, 0) * width_ + j) * 32 + 31];
			float sum02 = features_[(max(i - 1, 0) * width_ + min(j + 1, width_ - 1)) * 32 + 31];
			float sum10 = features_[(i * width_ + max(j - 1, 0)) * 32 + 31];
			float sum11 = features_[(i * width_ + j) * 32 + 31];
			float sum12 = features_[(i * width_ + min(j + 1, width_ - 1)) * 32 + 31];
			float sum20 = features_[(min(i + 1, height_ - 1) * width_ + max(j - 1, 0)) * 32 + 31];
			float sum21 = features_[(min(i + 1, height_ - 1) * width_ + j) * 32 + 31];
			float sum22 = features_[(min(i + 1, height_ - 1) * width_ + min(j + 1, width_ - 1)) * 32 + 31];

			float tmp[4];

			tmp[0] = sqrt(sum00 + sum01 + sum10 + sum11) + 1e-6f;
			tmp[1] = sqrt(sum01 + sum02 + sum11 + sum12) + 1e-6f;
			tmp[2] = sqrt(sum10 + sum11 + sum20 + sum21) + 1e-6f;
			tmp[3] = sqrt(sum11 + sum12 + sum21 + sum22) + 1e-6f;

			for (int k = 0; k < 4; ++k) {
				float sum = 0.0f;

				for (int l = 0; l < 9; ++l)
					sum += min(features_[(i * width_ + j) * 32 + l] / tmp[k], 0.2f);

				features_[(i * width_ + j) * 32 + 27 + k] = sum;
			}

			for (int k = 0; k < 27; ++k) {
				float sum = 0.0f;

				for (int l = 0; l < 4; ++l)
					sum += min(features_[(i * width_ + j) * 32 + k] / tmp[l], 0.2f);

				features_[(i * width_ + j) * 32 + k] = sum;
			}
		}
	}
}

scalar_t HoGHeuristic::computeFeature(unsigned int feature_index)
{
	// Compute the size of the region of interest
	const unsigned int roi_size = (roi_extent * 2 + 8) >> 3;

	const unsigned int bin = feature_index / (roi_size * roi_size);
	feature_index %= (roi_size * roi_size);

	const unsigned int i = feature_index / roi_size;
	const unsigned int j = feature_index % roi_size;

	const unsigned int x = (coordinates.x - roi_extent) >> 3;
	const unsigned int y = (coordinates.y - roi_extent) >> 3;

	return features_[((y + i) * width_ + (x + j)) * 32 + bin];
}

inline void HoGHeuristic::interpolate(int x, int y, int bin, float magnitude)
{
	// Find the bin into which (x, y) falls
	const int i = (y - 4) >> 3;
	const int j = (x - 4) >> 3;

	const int k = (y - 4) & 7;
	const int l = (x - 4) & 7;

	features_[(max(i, 0) * width_ + max(j, 0)) * 32 + bin] += magnitude * (7 - k) * (7 - l);
	features_[(max(i, 0) * width_ + min(j + 1, width_ - 1)) * 32 + bin] += magnitude * (7 - k) * l;
	features_[(min(i + 1, height_ - 1) * width_ + max(j, 0)) * 32 + bin] += magnitude * k * (7 - l);
	features_[(min(i + 1, height_ - 1) * width_ + min(j + 1, width_ - 1)) * 32 + bin] += magnitude * k * l;
}
