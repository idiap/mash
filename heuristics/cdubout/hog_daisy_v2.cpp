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
 *  Histogram of oriented gradients, inspired by the DAISY image descriptor.
 */

#include <mash/heuristic.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Mash;
using namespace std;

//------------------------------------------------------------------------------
/// The 'Daisy' heuristic class
//------------------------------------------------------------------------------
class DaisyHeuristic: public Heuristic
{
	//_____ Implementation of Heuristic __________
public:
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	static const int R = 15; // Distance from the center pixel to the outer most grid point
	static const int Q = 3; // Number of convolved orientations layers with different sigma's
	static const int H = 8; // Number of bins in the histogram

	// Gaussian blur an orientation map
	void blur(float* map, int width, int height, float sigma);

	// Orientation maps
	vector<float> maps_[Q][H];

	// The width and height of the orientation maps
	int width_;
	int height_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
	return new DaisyHeuristic();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int DaisyHeuristic::dim()
{
	// Compute the size of the region of interest
	const int roi_size = 2 * roi_extent + 1;

	int nbFeatures = 0;

	for (int i = 0; i < Q; ++i) {
		const int step = ((i + 1) * R) / (2.0f * Q);
		const int width = (roi_size - step / 2) / step;
		nbFeatures += width * width;
	}

	return H * nbFeatures;
}

void DaisyHeuristic::prepareForImage()
{
	// Fill the look-up tables
	float cosTheta[H];
	float sinTheta[H];

	for (int i = 0; i < H; ++i) {
		cosTheta[i] = cos(2.0f * i * static_cast<float>(M_PI) / H);
		sinTheta[i] = sin(2.0f * i * static_cast<float>(M_PI) / H);
	}

	// Get the dimensions of the image
	width_ = image->width();
	height_ = image->height();

	// Get the pixels values of the image
	RGBPixel_t **pixels = image->rgbLines();

	// Resize the first orientation maps
	for (int j = 0; j < H; ++j) {
		maps_[0][j].resize(width_ * height_);
		fill(maps_[0][j].begin(), maps_[0][j].end(), 0.0f);
	}

	// Compute the gradient at every pixel
	for (int y = 0; y < height_; ++y) {
		for (int x = 0; x < width_; ++x) {
			// Use the channel with the largest gradient magnitude
			const int xp = min(x + 1, width_ - 1);
			const int xm = max(x - 1, 0);
			const int yp = min(y + 1, height_ - 1);
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

			if ((gMagnitude >= rMagnitude) && (gMagnitude >= bMagnitude)) {
				for (int i = 0; i < H; ++i)
					maps_[0][i][y * width_ + x] += max(cosTheta[i] * gDx + sinTheta[i] * gDy, 0.0f);
			}
			else if (rMagnitude >= bMagnitude) {
				for (int i = 0; i < H; ++i)
					maps_[0][i][y * width_ + x] += max(cosTheta[i] * rDx + sinTheta[i] * rDy, 0.0f);
			}
			else if (bMagnitude) {
				for (int i = 0; i < H; ++i)
					maps_[0][i][y * width_ + x] += max(cosTheta[i] * bDx + sinTheta[i] * bDy, 0.0f);
			}
		}
	}

	// First map
	float previousSigma = R / (2.0f * Q);

	for (int i = 0; i < H; ++i)
		blur(&maps_[0][i][0], width_, height_, previousSigma);

	// Remaining maps
	for (int i = 1; i < Q; ++i) {
		float sigma = R * (i + 1) / (2.0f * Q);

		for (int j = 0; j < H; ++j) {
			maps_[i][j] = maps_[i - 1][j];
			blur(&maps_[i][j][0], width_, height_, sqrt(sigma * sigma - previousSigma * previousSigma));
		}

		previousSigma = sigma;
	}

	// Downscale and normalize maps
	for (int i = 0; i < Q; ++i) {
		const int step = ((i + 1) * R) / (2.0f * Q);
		const int width = (width_ - step / 2) / step;
		const int height = (height_ - step / 2) / step;

		for (int j = 0; j < H; ++j) {
			for (int y = 0; y < height; ++y)
				for (int x = 0; x < width; ++x)
					maps_[i][j][y * width + x] = maps_[i][j][(y * step + step / 2) * width_ + (x * step + step / 2)];

			maps_[i][j].resize(width * height);
		}

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				float sumSq = 0.0f;

				for (int j = 0; j < H; ++j)
					sumSq += maps_[i][j][y * width + x] * maps_[i][j][y * width + x];

				const float invNorm = 1.0f / sqrt(sumSq + 1e-6f);

				for (int j = 0; j < H; ++j)
					maps_[i][j][y * width + x] *= invNorm;
			}
		}
	}
}

scalar_t DaisyHeuristic::computeFeature(unsigned int feature_index)
{
	// Compute the size of the region of interest
	const int roi_size = 2 * roi_extent + 1;

	int nbFeatures = 0;

	for (int i = 0; i < Q; ++i) {
		const int step = ((i + 1) * R) / (2.0f * Q);
		const int width = (roi_size - step / 2) / step;
		const int stride = (width_ - step / 2) / step;

		for (int j = 0; j < H; ++j) {
			if (nbFeatures + width * width > feature_index) {
				feature_index -= nbFeatures;

				const int y = coordinates.y - roi_extent + (feature_index / width);
				const int x = coordinates.x - roi_extent + (feature_index % width);

				return maps_[i][j][y * stride + x];
			}

			nbFeatures += width * width;
		}
	}

/*	const int i = feature_index / (H * roi_size * roi_size);
	feature_index %= H * roi_size * roi_size;

	const int j = feature_index / (roi_size * roi_size);
	feature_index %= roi_size * roi_size;

	const int y = coordinates.y - roi_extent + (feature_index / roi_size);
	const int x = coordinates.x - roi_extent + (feature_index % roi_size);

	return maps_[i][j][y * width_ + x];*/
}

void DaisyHeuristic::blur(float* map, int width, int height, float sigma)
{
	// Half kernel length
	const int half = ceil(3.0f * sigma);

	// Gaussian kernel
	vector<float> kernel(2 * half + 1);

	for (int i = -half; i <= half; ++i)
		kernel[i + half] = exp(-(i * i) / (2.0f * sigma * sigma)) /
						   (sqrtf(2.0f * static_cast<float>(M_PI)) * sigma);

	// Temporary buffer
	vector<float> tmp(max(width, height));

	// Convolve rows
	for (int y = 0; y < height; ++y) {
		copy(map + y * width, map + (y + 1) * width, tmp.begin());

		for (int x = 0; x < half; ++x) {
			float sum = 0.0f;

			for (int k = -half; k <= half; ++k)
				sum += kernel[k + half] * tmp[min(max(x + k, 0), width - 1)];

			map[y * width + x] = sum;
		}

		for (int x = half; x < width - half; ++x) {
			float sum = 0.0f;

			for (int k = -half; k <= half; ++k)
				sum += kernel[k + half] * tmp[x + k];

			map[y * width + x] = sum;
		}

		for (int x = width - half; x < width; ++x) {
			float sum = 0.0f;

			for (int k = -half; k <= half; ++k)
				sum += kernel[k + half] * tmp[min(max(x + k, 0), width - 1)];

			map[y * width + x] = sum;
		}
	}

	// Convolve columns
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y)
			tmp[y] = map[y * width + x];

		for (int y = 0; y < half; ++y) {
			float sum = 0.0f;

			for (int k = -half; k <= half; ++k)
				sum += kernel[k + half] * tmp[min(max(y + k, 0), height - 1)];

			map[y * width + x] = sum;
		}

		for (int y = half; y < height - half; ++y) {
			float sum = 0.0f;

			for (int k = -half; k <= half; ++k)
				sum += kernel[k + half] * tmp[y + k];

			map[y * width + x] = sum;
		}

		for (int y = height - half; y < height; ++y) {
			float sum = 0.0f;

			for (int k = -half; k <= half; ++k)
				sum += kernel[k + half] * tmp[min(max(y + k, 0), height - 1)];

			map[y * width + x] = sum;
		}
	}
}
