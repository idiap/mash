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
 *  Pyramid of histograms of multi-block improved local binary patterns.
 *  Inspired from ftarsett/ilbp and created with the help of Cosmin Atanasoei.
 */

#include <mash/heuristic.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Mash;

//------------------------------------------------------------------------------
/// The 'MB_ILBP' heuristic class
//------------------------------------------------------------------------------
class MB_ILBP : public Heuristic {
	//_____ Implementation of Heuristic __________
public:
	MB_ILBP();
	virtual unsigned int dim();
	virtual void prepareForImage();
	virtual void finishForImage();
	virtual void prepareForCoordinates();
	virtual scalar_t computeFeature(unsigned int feature_index);

private:
	// The integral image
	std::vector<unsigned int> sat_;

	// The ILBP histograms
	std::vector<double> hists_[21]; // 21 = 1^2 + 2^2 + 4^2

	// The width of the current integral image
	unsigned int width_;
};

//------------------------------------------------------------------------------
/// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic() {
	return new MB_ILBP();
}

/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int MB_ILBP::dim() {
	return 21 * 256;
}

MB_ILBP::MB_ILBP() {
	for (int h = 0; h < 21; ++h) {
		hists_[h].resize(256);
	}
}

void MB_ILBP::prepareForImage() {
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

void MB_ILBP::finishForImage() {
	// Clear the integral image
	sat_.clear();
}

// Returns the area of the intersection between a rectangle and a square
struct Intersector {
	Intersector(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
	: x_(x), y_(y), w_(w), h_(h) {}

	unsigned int operator()(unsigned int x, unsigned int y, unsigned int w) const {
		const unsigned int x1 = std::max(x_, x);
		const unsigned int x2 = std::min(x_ + w_, x + w);

		if (x1 >= x2) {
			return 0;
		}

		const unsigned int y1 = std::max(y_, y);
		const unsigned int y2 = std::min(y_ + h_, y + w);

		if (y1 >= y2) {
			return 0;
		}

		return (x2 - x1) * (y2 - y1);
	}

	const unsigned int x_, y_, w_, h_;
};

void MB_ILBP::prepareForCoordinates() {
	// Compute the size of the region of interest
	const unsigned int roi_size = 2 * roi_extent + 1;

	// Clear the histograms
	for (int h = 0; h < 21; ++h) {
		std::fill(hists_[h].begin(), hists_[h].end(), 0.0);
	}

	// Iterate over all the possible cell sizes
	for (unsigned int ch = 1; ch <= roi_size / 3; ch <<= 1) {
		for (unsigned int cw = 1; cw <= roi_size / 3; cw <<= 1) {
			// Iterate over all the possible cell positions
			for (unsigned int cy = 0; cy <= roi_size - 3 * ch; cy += (ch + 3) >> 2) {
				for (unsigned int cx = 0; cx <= roi_size - 3 * cw; cx += (cw + 3) >> 2) {
					// Coordinates of the upper left corner of the LBP
					const unsigned int x = coordinates.x - roi_extent + cx;
					const unsigned int y = coordinates.y - roi_extent + cy;

					unsigned int s[4][4];

					for (unsigned int i = 0; i < 4; ++i) {
						for (unsigned int j = 0; j < 4; ++j) {
							s[i][j] = sat_[(y + i * ch) * width_ + x + j * cw];
						}
					}

					const double avg = (s[0][0] - s[0][3] - s[3][0] + s[3][3]) / 9.0;

					for (unsigned int i = 0; i < 3; ++i) {
						for (unsigned int j = 0; j < 3; ++j) {
							s[i][j] += s[i + 1][j + 1] - s[i][j + 1] - s[i + 1][j];
						}
					}

					const unsigned int code = ((s[0][0] > avg)     ) |
											  ((s[0][1] > avg) << 1) |
											  ((s[0][2] > avg) << 2) |
											  ((s[1][0] > avg) << 3) |
											  ((s[1][2] > avg) << 4) |
											  ((s[2][0] > avg) << 5) |
											  ((s[2][1] > avg) << 6) |
											  ((s[2][2] > avg) << 7);

					const double dist = (std::abs(s[0][0] - avg) +
										 std::abs(s[0][1] - avg) +
										 std::abs(s[0][2] - avg) +
										 std::abs(s[1][0] - avg) +
										 std::abs(s[1][1] - avg) +
										 std::abs(s[1][2] - avg) +
										 std::abs(s[2][0] - avg) +
										 std::abs(s[2][1] - avg) +
										 std::abs(s[2][2] - avg)) / (81.0 * cw * cw * ch * ch);

					Intersector intersector(cx, cy, 3 * cw, 3 * ch);

					// Level 0 of the pyramid
					hists_[0][code] += dist;

					// Level 1 of the pyramid
					const unsigned int half = roi_extent + 1;
					hists_[1][code] += intersector(0, 0, half) * dist;
					hists_[2][code] += intersector(half, 0, half) * dist;
					hists_[3][code] += intersector(0, half, half) * dist;
					hists_[4][code] += intersector(half, half, half) * dist;

					// Level 2 of the pyramid
					const unsigned int quarterDown = (roi_extent >> 1) + 1;
					const unsigned int quarterUp = (roi_extent + 1) >> 1;
					hists_[ 5][code] += intersector(				 0,					 0,   quarterUp) * dist;
					hists_[ 6][code] += intersector(	 quarterUp + 1,					 0, quarterDown) * dist;
					hists_[ 7][code] += intersector(			  half,					 0, quarterDown) * dist;
					hists_[ 8][code] += intersector(half + quarterDown,					 0,   quarterUp) * dist;
					hists_[ 8][code] += intersector(				 0,		 quarterUp + 1,   quarterUp) * dist;
					hists_[10][code] += intersector(	 quarterUp + 1,		 quarterUp + 1, quarterDown) * dist;
					hists_[11][code] += intersector(			  half,		 quarterUp + 1, quarterDown) * dist;
					hists_[12][code] += intersector(half + quarterDown,		 quarterUp + 1,   quarterUp) * dist;
					hists_[13][code] += intersector(				 0,				  half,   quarterUp) * dist;
					hists_[14][code] += intersector(	 quarterUp + 1,				  half, quarterDown) * dist;
					hists_[15][code] += intersector(			  half,				  half, quarterDown) * dist;
					hists_[16][code] += intersector(half + quarterDown,				  half,   quarterUp) * dist;
					hists_[17][code] += intersector(				 0, half + quarterDown,   quarterUp) * dist;
					hists_[18][code] += intersector(	 quarterUp + 1, half + quarterDown, quarterDown) * dist;
					hists_[19][code] += intersector(			  half, half + quarterDown, quarterDown) * dist;
					hists_[20][code] += intersector(half + quarterDown, half + quarterDown,   quarterUp) * dist;
				}
			}
		}
	}
}

scalar_t MB_ILBP::computeFeature(unsigned int feature_index) {
	return hists_[feature_index / 256][feature_index % 256];
}
