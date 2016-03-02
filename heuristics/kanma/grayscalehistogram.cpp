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

    This heuristic computes the histogram of the region of interest, in grayscale
    format
*/

#include <mash/heuristic.h>
#include <memory.h>
#include <assert.h>

using namespace Mash;


/******************************* HISTOGRAM CLASS ******************************/

//------------------------------------------------------------------------------
// Declaration of a histogram class specialized in grayscale images
//------------------------------------------------------------------------------
class GrayscaleHistogram
{
    //_____ Construction / Destruction __________
public:
    GrayscaleHistogram(unsigned int nbBins);
    ~GrayscaleHistogram();
    
    
    //_____ Methods __________
public:
    void compute(Image* pImage, unsigned int start_x, unsigned int start_y,
                 unsigned int width, unsigned int height, bool bClear = true);
    void clear();
    
    inline unsigned int* bins()
    {
        return _bins;
    }

    inline unsigned int nbBins()
    {
        return _nbBins;
    }
    
    
    //_____ Attributes __________
private:
    unsigned int    _nbBins;
    unsigned int*   _bins;
    unsigned int    _lookup[256];
};


GrayscaleHistogram::GrayscaleHistogram(unsigned int nbBins)
: _nbBins(nbBins), _bins(0)
{
	// Assertions
	assert(nbBins > 0);
	
	_bins = new unsigned int[nbBins];
	clear();
    
    unsigned int bin_size = 256 / nbBins;
    for (unsigned int i = 0; i < 256; ++i)
        _lookup[i] = i / bin_size;
}


GrayscaleHistogram::~GrayscaleHistogram()
{
	delete[] _bins;
}


void GrayscaleHistogram::compute(Image* pImage, unsigned int start_x,
                                 unsigned int start_y, unsigned int width,
                                 unsigned int height, bool bClear)
{
	// Assertions
	assert(pImage);
	assert(width > 0);
	assert(height > 0);
	assert(start_x < pImage->width());
	assert(start_y < pImage->height());
	assert(width + start_x <= pImage->width());
	assert(height + start_y <= pImage->height());
	assert(_nbBins > 0);
	assert(_bins);
	
	if (bClear)
		clear();
    
    byte_t** pLines = pImage->grayLines();
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
            unsigned int index = _lookup[pLines[start_y + y][start_x + x]];
			_bins[index]++;
		}
	}
}


void GrayscaleHistogram::clear()
{
	// Assertions
	assert(_nbBins > 0);
	assert(_bins);
	
	memset(_bins, 0, _nbBins * sizeof(unsigned int));
}


/******************************* HEURISTIC CLASS ******************************/

//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class GrayscaleHistogramHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    GrayscaleHistogramHeuristic();
    virtual ~GrayscaleHistogramHeuristic();


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim();

    virtual void prepareForCoordinates();

    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:
    GrayscaleHistogram _histogram;
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new GrayscaleHistogramHeuristic();
}


GrayscaleHistogramHeuristic::GrayscaleHistogramHeuristic()
: _histogram(256)
{
}


GrayscaleHistogramHeuristic::~GrayscaleHistogramHeuristic()
{
}


unsigned int GrayscaleHistogramHeuristic::dim()
{
    return _histogram.nbBins();
}


void GrayscaleHistogramHeuristic::prepareForCoordinates()
{
    // Compute the coordinates of the region of interest
    unsigned int x0 = coordinates.x - roi_extent;
    unsigned int y0 = coordinates.y - roi_extent;
    unsigned int roi_size = roi_extent * 2 + 1;

    // Compute the histogram
    _histogram.compute(image, x0, y0, roi_size, roi_size, true);
}


scalar_t GrayscaleHistogramHeuristic::computeFeature(unsigned int feature_index)
{
    return _histogram.bins()[feature_index];
}
