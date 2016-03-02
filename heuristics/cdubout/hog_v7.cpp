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
	static const unsigned int NB_BINS  = 12;

	// Type of a rectangle
	struct Rect {
		// Coordinates of the region where to compute the histogram
		int xMin, yMin, xMax, yMax;

		// The bin the rectangle is looking at
		unsigned int bin;
	};

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

	// Randomize the nbRects rectangles
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
	static const scalar_t linear[256] = {
		0.000000f, 0.000304f, 0.000607f, 0.000911f, 0.001214f, 0.001518f, 0.001821f, 0.002125f,
		0.002428f, 0.002732f, 0.003035f, 0.003347f, 0.003677f, 0.004025f, 0.004391f, 0.004777f,
		0.005182f, 0.005605f, 0.006049f, 0.006512f, 0.006995f, 0.007499f, 0.008023f, 0.008568f,
		0.009134f, 0.009721f, 0.010330f, 0.010960f, 0.011612f, 0.012286f, 0.012983f, 0.013702f,
		0.014444f, 0.015209f, 0.015996f, 0.016807f, 0.017642f, 0.018500f, 0.019382f, 0.020289f,
		0.021219f, 0.022174f, 0.023153f, 0.024158f, 0.025187f, 0.026241f, 0.027321f, 0.028426f,
		0.029557f, 0.030713f, 0.031896f, 0.033105f, 0.034340f, 0.035601f, 0.036889f, 0.038204f,
		0.039546f, 0.040915f, 0.042311f, 0.043735f, 0.045186f, 0.046665f, 0.048172f, 0.049707f,
		0.051269f, 0.052861f, 0.054480f, 0.056128f, 0.057805f, 0.059511f, 0.061246f, 0.063010f,
		0.064803f, 0.066626f, 0.068478f, 0.070360f, 0.072272f, 0.074214f, 0.076185f, 0.078187f,
		0.080220f, 0.082283f, 0.084376f, 0.086500f, 0.088656f, 0.090842f, 0.093059f, 0.095307f,
		0.097587f, 0.099899f, 0.102242f, 0.104616f, 0.107023f, 0.109462f, 0.111932f, 0.114435f,
		0.116971f, 0.119538f, 0.122139f, 0.124772f, 0.127438f, 0.130136f, 0.132868f, 0.135633f,
		0.138432f, 0.141263f, 0.144128f, 0.147027f, 0.149960f, 0.152926f, 0.155926f, 0.158961f,
		0.162029f, 0.165132f, 0.168269f, 0.171441f, 0.174647f, 0.177888f, 0.181164f, 0.184475f,
		0.187821f, 0.191202f, 0.194618f, 0.198069f, 0.201556f, 0.205079f, 0.208637f, 0.212231f,
		0.215861f, 0.219526f, 0.223228f, 0.226966f, 0.230740f, 0.234551f, 0.238398f, 0.242281f,
		0.246201f, 0.250158f, 0.254152f, 0.258183f, 0.262251f, 0.266356f, 0.270498f, 0.274677f,
		0.278894f, 0.283149f, 0.287441f, 0.291771f, 0.296138f, 0.300544f, 0.304987f, 0.309469f,
		0.313989f, 0.318547f, 0.323143f, 0.327778f, 0.332452f, 0.337164f, 0.341914f, 0.346704f,
		0.351533f, 0.356400f, 0.361307f, 0.366253f, 0.371238f, 0.376262f, 0.381326f, 0.386429f,
		0.391572f, 0.396755f, 0.401978f, 0.407240f, 0.412543f, 0.417885f, 0.423268f, 0.428690f,
		0.434154f, 0.439657f, 0.445201f, 0.450786f, 0.456411f, 0.462077f, 0.467784f, 0.473531f,
		0.479320f, 0.485150f, 0.491021f, 0.496933f, 0.502886f, 0.508881f, 0.514918f, 0.520996f,
		0.527115f, 0.533276f, 0.539479f, 0.545724f, 0.552011f, 0.558340f, 0.564712f, 0.571125f,
		0.577580f, 0.584078f, 0.590619f, 0.597202f, 0.603827f, 0.610496f, 0.617207f, 0.623960f,
		0.630757f, 0.637597f, 0.644480f, 0.651406f, 0.658375f, 0.665387f, 0.672443f, 0.679542f,
		0.686685f, 0.693872f, 0.701102f, 0.708376f, 0.715694f, 0.723055f, 0.730461f, 0.737910f,
		0.745404f, 0.752942f, 0.760525f, 0.768151f, 0.775822f, 0.783538f, 0.791298f, 0.799103f,
		0.806952f, 0.814847f, 0.822786f, 0.830770f, 0.838799f, 0.846873f, 0.854993f, 0.863157f,
		0.871367f, 0.879622f, 0.887923f, 0.896269f, 0.904661f, 0.913099f, 0.921582f, 0.930111f,
		0.938686f, 0.947307f, 0.955973f, 0.964686f, 0.973445f, 0.982251f, 0.991102f, 1.000000f,
	};

	// Get the size of the image
	const unsigned int width = image->width();
	const unsigned int height = image->height();

	// Initialize the integral images
	for(unsigned int b = 0; b < NB_BINS; ++b) {
		sats_[b].resize(width * height, 0);
	}

	// Get the pixels values of the image
	byte_t** pixels = image->grayLines();

	// Compute the gradient at every pixel
	for(unsigned int y = 1; y < height - 1; ++y) {
		for(unsigned int x = 1; x < width - 1; ++x) {
			scalar_t dx = linear[pixels[y][x + 1]] - linear[pixels[y][x - 1]];
			scalar_t dy = linear[pixels[y + 1][x]] - linear[pixels[y - 1][x]];

			// Compute the magnitude and the orientation
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
