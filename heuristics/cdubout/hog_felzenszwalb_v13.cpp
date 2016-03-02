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
	// small value, used to avoid division by zero
	const double eps = 0.0001f;

	// unit vectors used to compute gradient orientation
	const float uu[9] = {1.0000f, 0.9397f, 0.7660f, 0.5000f, 0.1736f,-0.1736f,-0.5000f,-0.7660f,-0.9397f};
	const float vv[9] = {0.0000f, 0.3420f, 0.6428f, 0.8660f, 0.9848f, 0.9848f, 0.8660f, 0.6428f, 0.3420f};

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
			float dx = 0.0f;
			float dy = 0.0f;

			if (rMagnitude > gMagnitude && rMagnitude > bMagnitude) {
				magnitude = sqrtf(rMagnitude);
				dx = rDx;
				dy = rDy;
			}
			else if (gMagnitude > bMagnitude) {
				magnitude = sqrtf(gMagnitude);
				dx = gDx;
				dy = gDy;
			}
			else {
				magnitude = sqrtf(bMagnitude);
				dx = bDx;
				dy = bDy;
			}
			
			// Snap to one of 18 orientations
			float best_dot = 0.0f;
			int best_o = 0;
	
			for (int o = 0; o < 9; o++) {
				const float dot = uu[o] * dx + vv[o] * dy;

				if (dot > best_dot) {
					best_dot = dot;
					best_o = o;
				}
				else if (-dot > best_dot) {
					best_dot = -dot;
					best_o = o + 9;
				}
			}

			// Add to 4 histograms around pixel using linear interpolation
			float xp = (x + 0.5f) / 8.0f - 0.5f;
			float yp = (y + 0.5f) / 8.0f - 0.5f;
			int ixp = floorf(xp);
			int iyp = floorf(yp);
			float vx0 = xp - ixp;
			float vy0 = yp - iyp;
			float vx1 = 1.0f - vx0;
			float vy1 = 1.0f - vy0;

			if (ixp >= 0 && iyp >= 0)
				features_[(iyp * width_ + ixp) * 32 + best_o] += vx1 * vy1 * magnitude;

			if (ixp + 1 < width_ && iyp >= 0)
				features_[(iyp * width_ + ixp + 1) * 32 + best_o] += vx0 * vy1 * magnitude;

			if (ixp >= 0 && iyp + 1 < height_)
				features_[((iyp + 1) * width_ + ixp) * 32 + best_o] += vx1 * vy0 * magnitude;

			if (ixp + 1 < width_ && iyp + 1 < height_)
				features_[((iyp + 1) * width_ + ixp + 1) * 32 + best_o] += vx0 * vy0 * magnitude;
		}
	}

	// Compute energy in each block by summing over orientations
	for (int i = 0; i < height_; ++i) {
		for (int j = 0; j < width_; ++j) {
			float sumSquare = 0.0f;

			for (int k = 0; k < 18; ++k)
				sumSquare += features_[(i * width_ + j) * 32 + k] * features_[(i * width_ + j) * 32 + k];

			features_[(i * width_ + j) * 32 + 31] = sumSquare;
		}
	}

	// compute features
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

			float n1 = 1.0f / sqrtf(sum00 + sum01 + sum10 + sum11 + eps);
			float n2 = 1.0f / sqrtf(sum01 + sum02 + sum11 + sum12 + eps);
			float n3 = 1.0f / sqrtf(sum10 + sum11 + sum20 + sum21 + eps);
			float n4 = 1.0f / sqrtf(sum11 + sum12 + sum21 + sum22 + eps);

			// contrast-insensitive features
			for (int o = 0; o < 9; o++) {
				float sum = features_[(i * width_ + j) * 32 + o] + features_[(i * width_ + j) * 32 + o + 9];
				float h1 = min(sum * n1, 0.2f);
				float h2 = min(sum * n2, 0.2f);
				float h3 = min(sum * n3, 0.2f);
				float h4 = min(sum * n4, 0.2f);
				features_[(i * width_ + j) * 32 + o + 18] = 0.5f * (h1 + h2 + h3 + h4);
			}

			float t1 = 0.0f;
			float t2 = 0.0f;
			float t3 = 0.0f;
			float t4 = 0.0f;

			// Contrast-sensitive features
			for (int o = 0; o < 18; o++) {
				float h1 = min(features_[(i * width_ + j) * 32 + o] * n1, 0.2f);
				float h2 = min(features_[(i * width_ + j) * 32 + o] * n2, 0.2f);
				float h3 = min(features_[(i * width_ + j) * 32 + o] * n3, 0.2f);
				float h4 = min(features_[(i * width_ + j) * 32 + o] * n4, 0.2f);
				features_[(i * width_ + j) * 32 + o] = 0.5f * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
			}

			// Texture features
			features_[(i * width_ + j) * 32 + 27] = 0.2357f * t1;
			features_[(i * width_ + j) * 32 + 28] = 0.2357f * t2;
			features_[(i * width_ + j) * 32 + 29] = 0.2357f * t3;
			features_[(i * width_ + j) * 32 + 30] = 0.2357f * t4;
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
