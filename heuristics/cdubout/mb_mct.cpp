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
 *  Histogram of multi-block MCT in the whole image.
 *  Inspired from ftarsett/ilbp and by Cosmin Atanasoei.
 */

#include <mash/heuristic.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'MB_MCT' heuristic class
//------------------------------------------------------------------------------
class MB_MCT : public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	MB_MCT();
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual void finishForImage();
	virtual void prepareForCoordinates();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// The integral image
	std::vector<unsigned int> sat_;

	// The MCT histogram
	std::vector<unsigned int> hist_;

	// The width of the current integral image
	unsigned int width_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new MB_MCT();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int MB_MCT::dim() {
	return 512;
}

MB_MCT::MB_MCT() : hist_(512) {}

void MB_MCT::prepareForImage() {
	// Get the size of the image
	const unsigned int width = image->width();
	const unsigned int height = image->height();

	// Resize the integral image to the size of the image
	sat_.resize((width + 1) * (height + 1), 0U);

	// Save the width of the current integral image
	width_ = width + 1;

	// Get the pixels values of the image
	byte_t** pixels = image->grayLines();

	// Recopy the original into the integral image
	for (unsigned int y = 0; y < height; ++y) {
		for (unsigned int x = 0; x < width; ++x) {
			sat_[(y + 1) * width_ + x + 1] = pixels[y][x];
		}
	}

	// Integral image formula: sat(D) = i(D) + sat(B) + sat(D) - sat(A)
	for (unsigned int y = 1; y <= height; ++y) {
		for (unsigned int x = 1; x <= width; ++x) {
			sat_[y * width_ + x] += sat_[y * width_ + x - 1] +
									sat_[(y - 1) * width_ + x] -
									sat_[(y - 1) * width_ + x - 1];
		}
	}
}

void MB_MCT::finishForImage() {
	// Clear the integral image
	sat_.clear();
}

void MB_MCT::prepareForCoordinates() {
	// Compute the size of the region of interest
	const unsigned int roi_size = 2 * roi_extent + 1;

	// Clear the histogram
	std::fill(hist_.begin(), hist_.end(), 0U);

	// Iterate over all the possible cell sizes
	for (unsigned int ch = 1U; ch <= roi_size / 3; ch <<= 1) {
		for (unsigned int cw = 1U; cw <= roi_size / 3; cw <<= 1) {
			// Iterate over all the possible cell positions
			for (unsigned int cy = 0; cy <= roi_size - 3 * ch; cy += ch) {
				for (unsigned int cx = 0; cx <= roi_size - 3 * cw; cx += cw) {
					const unsigned int x = coordinates.x - roi_extent + cx;
					const unsigned int y = coordinates.y - roi_extent + cy;

					unsigned int s[4][4];

					for (unsigned int i = 0; i < 4; ++i) {
						for (unsigned int j = 0; j < 4; ++j) {
							s[i][j] = sat_[(y + i * ch) * width_ + x + j * cw];
						}
					}

					const unsigned int avg = (s[0][0] - s[0][3] - s[3][0] + s[3][3]) / 9;

					++hist_[(((s[0][0] - s[0][1] - s[1][0] + s[1][1]) > avg)     ) |
							(((s[0][1] - s[0][2] - s[1][1] + s[1][2]) > avg) << 1) |
							(((s[0][2] - s[0][3] - s[1][2] + s[1][3]) > avg) << 2) |
							(((s[1][0] - s[1][1] - s[2][0] + s[2][1]) > avg) << 3) |
							(((s[1][1] - s[1][2] - s[2][1] + s[2][2]) > avg) << 4) |
							(((s[1][2] - s[1][3] - s[2][2] + s[2][3]) > avg) << 5) |
							(((s[2][0] - s[2][1] - s[3][0] + s[3][1]) > avg) << 6) |
							(((s[2][1] - s[2][2] - s[3][1] + s[3][2]) > avg) << 7) |
							(((s[2][2] - s[2][3] - s[3][2] + s[3][3]) > avg) << 8)];
				}
			}
		}
	}
}

scalar_t MB_MCT::computeFeature(unsigned int feature_index) {
	return hist_[feature_index];
}
