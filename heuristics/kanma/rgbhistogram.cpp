/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Philip Abbet (philip.abbet@idiap.ch)
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


/** Author: Philip Abbet (philip.abbet@idiap.ch)

    This heuristic computes the histogram of the region of interest, in RGB
    format
*/

#include <mash/heuristic.h>
#include <memory.h>
#include <assert.h>

using namespace Mash;


/******************************* HISTOGRAM CLASS ******************************/

//------------------------------------------------------------------------------
// Declaration of a histogram class specialized in RGB images
//------------------------------------------------------------------------------
class RGBHistogram
{
    //_____ Construction / Destruction __________
public:
    RGBHistogram(unsigned int nbBinsPerChannel);
    ~RGBHistogram();
    
    
    //_____ Methods __________
public:
    void compute(Image* pImage, unsigned int start_x, unsigned int start_y,
                 unsigned int width, unsigned int height, bool bClear = true);
    void clear();
    
    inline unsigned int* bins()
    {
        return _bins;
    }

    inline unsigned int nbBinsTotal()
    {
        return _nbBinsTotal;
    }
    
    
    //_____ Attributes __________
private:
    unsigned int    _nbBinsPerChannel;
    unsigned int    _nbBinsTotal;
    unsigned int*   _bins;
    unsigned int    _lookup[256];
};


RGBHistogram::RGBHistogram(unsigned int nbBinsPerChannel)
: _nbBinsPerChannel(nbBinsPerChannel), _nbBinsTotal(nbBinsPerChannel * nbBinsPerChannel * nbBinsPerChannel),
  _bins(0)
{
	// Assertions
	assert(nbBinsPerChannel > 0);
	
	_bins = new unsigned int[_nbBinsTotal];
	clear();
    
    unsigned int bin_size = 256 / nbBinsPerChannel;
    for (unsigned int i = 0; i < 256; ++i)
        _lookup[i] = i / bin_size;
}


RGBHistogram::~RGBHistogram()
{
	delete[] _bins;
}


void RGBHistogram::compute(Image* pImage, unsigned int start_x, unsigned int start_y,
                           unsigned int width, unsigned int height, bool bClear)
{
	// Assertions
	assert(pImage);
	assert(width > 0);
	assert(height > 0);
	assert(start_x < pImage->width());
	assert(start_y < pImage->height());
	assert(width + start_x <= pImage->width());
	assert(height + start_y <= pImage->height());
	assert(_nbBinsTotal > 0);
	assert(_bins);
	
	if (bClear)
		clear();
    
    RGBPixel_t** pLines = pImage->rgbLines();
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
            RGBPixel_t pixel = pLines[start_y + y][start_x + x];
			unsigned int index = _lookup[pixel.r] + _lookup[pixel.g] * _nbBinsPerChannel +
                                 _lookup[pixel.b] * _nbBinsPerChannel * _nbBinsPerChannel;
			_bins[index]++;
		}
	}
}


void RGBHistogram::clear()
{
	// Assertions
	assert(_nbBinsTotal > 0);
	assert(_bins);
	
	memset(_bins, 0, _nbBinsTotal * sizeof(unsigned int));
}


/******************************* HEURISTIC CLASS ******************************/

//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class RGBHistogramHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    RGBHistogramHeuristic();
    virtual ~RGBHistogramHeuristic();


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim();

    virtual void prepareForCoordinates();

    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:
    RGBHistogram _histogram;
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new RGBHistogramHeuristic();
}


RGBHistogramHeuristic::RGBHistogramHeuristic()
: _histogram(8)
{
}


RGBHistogramHeuristic::~RGBHistogramHeuristic()
{
}


unsigned int RGBHistogramHeuristic::dim()
{
    return _histogram.nbBinsTotal();
}


void RGBHistogramHeuristic::prepareForCoordinates()
{
    // Compute the coordinates of the region of interest
    unsigned int x0 = coordinates.x - roi_extent;
    unsigned int y0 = coordinates.y - roi_extent;
    unsigned int roi_size = roi_extent * 2 + 1;

    // Compute the histogram
    _histogram.compute(image, x0, y0, roi_size, roi_size, true);
}


scalar_t RGBHistogramHeuristic::computeFeature(unsigned int feature_index)
{
    return _histogram.bins()[feature_index];
}
