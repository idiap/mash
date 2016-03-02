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
	return ((roi_size + 7) >> 3) * ((roi_size + 7) >> 3) * 31;
}

void HoGHeuristic::prepareForImage()
{
	// Table of all the possible tangents (1MB)
	static float ATAN2_TABLE[512][512] = {{0.0f}};

	// Fill the atan2 table
	if (ATAN2_TABLE[0][0] == 0.0f) {
		for (int dy = -255; dy <= 255; ++dy) {
			for (int dx = -255; dx <= 255; ++dx) {
				// Angle in the range [-pi, pi]
				float angle = atan2f(dy, dx);

				// Convert it to the range [0, 18)
				angle = max(angle * static_cast<float>(9.0 / M_PI) + 9.0f, 0.0f);

				if (angle >= 18.0f)
					angle = 0.0f;

				ATAN2_TABLE[dy + 255][dx + 255] = angle;
			}
		}
	}

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
			const int xp = min(x + 1, width - 1);
			const int xm = max(x - 1, 0);
			const int yp = min(y + 1, height - 1);
			const int ym = max(y - 1, 0);

			const int rDx = static_cast<int>(pixels[y][xp].r) - static_cast<int>(pixels[y][xm].r);
			const int rDy = static_cast<int>(pixels[yp][x].r) - static_cast<int>(pixels[ym][x].r);
			const int gDx = static_cast<int>(pixels[y][xp].g) - static_cast<int>(pixels[y][xm].g);
			const int gDy = static_cast<int>(pixels[yp][x].g) - static_cast<int>(pixels[ym][x].g);
			const int bDx = static_cast<int>(pixels[y][xp].b) - static_cast<int>(pixels[y][xm].b);
			const int bDy = static_cast<int>(pixels[yp][x].b) - static_cast<int>(pixels[ym][x].b);

			const int rMagnitude = rDx * rDx + rDy * rDy;
			const int gMagnitude = gDx * gDx + gDy * gDy;
			const int bMagnitude = bDx * bDx + bDy * bDy;

			float magnitude = 0.0f;
			float theta = 0.0f;

			if ((gMagnitude >= rMagnitude) && (gMagnitude >= bMagnitude)) {
				magnitude = sqrtf(rMagnitude);
				theta = ATAN2_TABLE[gDy + 255][gDx + 255];
			}
			else if (rMagnitude >= bMagnitude) {
				magnitude = sqrtf(gMagnitude);
				theta = ATAN2_TABLE[rDy + 255][rDx + 255];
			}
			else if (bMagnitude) {
				magnitude = sqrtf(bMagnitude);
				theta = ATAN2_TABLE[bDy + 255][bDx + 255];
			}

			// Discretize theta
			const int theta0 = theta;
			const int theta1 = (theta0 < 17) ? (theta0 + 1) : 0;
			const float alpha = theta - theta0;

			interpolate(x, y, theta0, magnitude * (1.0f - alpha));
			interpolate(x, y, theta1, magnitude * alpha);
		}
	}

	// Compute the "gradient energy" of each cell, i.e. ||C(i,j)||^2
	for (int i = 0; i < height_; ++i) {
		for (int j = 0; j < width_; ++j) {
			float sumSq = 0.0f;

			for (int k = 0; k < 9; ++k)
				sumSq += (features_[(i * width_ + j) * 32 + k] + features_[(i * width_ + j) * 32 + k + 9]) *
						 (features_[(i * width_ + j) * 32 + k] + features_[(i * width_ + j) * 32 + k + 9]);

			features_[(i * width_ + j) * 32 + 31] = sumSq;
		}
	}

	// Compute the four normalization factors and normalize and clamp everything
	for (int i = 0; i < height_; ++i) {
		for (int j = 0; j < width_; ++j) {
			const float sum00 = features_[(max(i - 1, 0) * width_ + max(j - 1, 0)) * 32 + 31];
			const float sum01 = features_[(max(i - 1, 0) * width_ + j) * 32 + 31];
			const float sum02 = features_[(max(i - 1, 0) * width_ + min(j + 1, width_ - 1)) * 32 + 31];
			const float sum10 = features_[(i * width_ + max(j - 1, 0)) * 32 + 31];
			const float sum11 = features_[(i * width_ + j) * 32 + 31];
			const float sum12 = features_[(i * width_ + min(j + 1, width_ - 1)) * 32 + 31];
			const float sum20 = features_[(min(i + 1, height_ - 1) * width_ + max(j - 1, 0)) * 32 + 31];
			const float sum21 = features_[(min(i + 1, height_ - 1) * width_ + j) * 32 + 31];
			const float sum22 = features_[(min(i + 1, height_ - 1) * width_ + min(j + 1, width_ - 1)) * 32 + 31];

			// Normalization factors
			const float n0 = 1.0f / sqrtf(sum00 + sum01 + sum10 + sum11 + 1e-6f);
			const float n1 = 1.0f / sqrtf(sum01 + sum02 + sum11 + sum12 + 1e-6f);
			const float n2 = 1.0f / sqrtf(sum10 + sum11 + sum20 + sum21 + 1e-6f);
			const float n3 = 1.0f / sqrtf(sum11 + sum12 + sum21 + sum22 + 1e-6f);

			// Contrast-insensitive features
			for (int k = 0; k < 9; ++k) {
				const float sum = features_[(i * width_ + j) * 32 + k] + features_[(i * width_ + j) * 32 + k + 9];
				const float h0 = min(sum * n0, 0.2f);
				const float h1 = min(sum * n1, 0.2f);
				const float h2 = min(sum * n2, 0.2f);
				const float h3 = min(sum * n3, 0.2f);
				features_[(i * width_ + j) * 32 + k + 18] = (h0 + h1 + h2 + h3) / 0.8f;
			}

			// Contrast-sensitive features
			float t0 = 0.0f;
			float t1 = 0.0f;
			float t2 = 0.0f;
			float t3 = 0.0f;

			for (int k = 0; k < 18; ++k) {
				const float sum = features_[(i * width_ + j) * 32 + k];
				const float h0 = min(sum * n0, 0.2f);
				const float h1 = min(sum * n1, 0.2f);
				const float h2 = min(sum * n2, 0.2f);
				const float h3 = min(sum * n3, 0.2f);
				features_[(i * width_ + j) * 32 + k] = (h0 + h1 + h2 + h3) / 0.8f;
				t0 += h0;
				t1 += h1;
				t2 += h2;
				t3 += h3;
			}

			features_[(i * width_ + j) * 32 + 27] = t0 / 3.6f;
			features_[(i * width_ + j) * 32 + 28] = t1 / 3.6f;
			features_[(i * width_ + j) * 32 + 29] = t2 / 3.6f;
			features_[(i * width_ + j) * 32 + 30] = t3 / 3.6f;
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

	// Bilinear interpolation
	const int a = k * 2 + 1;
	const int b = 16 - a;
	const int c = l * 2 + 1;
	const int d = 16 - c;

	features_[(max(i, 0) * width_ + max(j, 0)) * 32 + bin] += magnitude * (b * d);
	features_[(max(i, 0) * width_ + min(j + 1, width_ - 1)) * 32 + bin] += magnitude * (b * c);
	features_[(min(i + 1, height_ - 1) * width_ + max(j, 0)) * 32 + bin] += magnitude * (a * d);
	features_[(min(i + 1, height_ - 1) * width_ + min(j + 1, width_ - 1)) * 32 + bin] += magnitude * (a * c);
}
