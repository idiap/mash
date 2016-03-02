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

    This heuristic produces a 'blobbed' version of the (RGB) region of interest.
    The computation of the blobs is done by segmenting the RGB space in 512
    discrete values (each channel range is segmented in 8).
*/

#include <mash/heuristic.h>
#include <memory.h>

using namespace Mash;


//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class BlobsHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    BlobsHeuristic();
    virtual ~BlobsHeuristic();


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim();

    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:
    byte_t _lookup[256];
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new BlobsHeuristic();
}


/************************* CONSTRUCTION / DESTRUCTION *************************/

BlobsHeuristic::BlobsHeuristic()
{
    for (unsigned int i = 0; i < 256; ++i)
        _lookup[i] = i / 32;
}


BlobsHeuristic::~BlobsHeuristic()
{
}


/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int BlobsHeuristic::dim()
{
    // We have has many features than pixels in the region of interest
    unsigned int roi_size = roi_extent * 2 + 1;
    return roi_size * roi_size;
}


scalar_t BlobsHeuristic::computeFeature(unsigned int feature_index)
{
    // Compute the coordinates of the top-left pixel of the region of interest
    unsigned int x0 = coordinates.x - roi_extent;
    unsigned int y0 = coordinates.y - roi_extent;

    // Compute the coordinates of the pixel corresponding to the feature, in
    // the region of interest
    unsigned int roi_size = roi_extent * 2 + 1;
    unsigned int x = feature_index % roi_size;
    unsigned int y = (feature_index - x) / roi_size;

    // Return the blobbed value of the pixel corresponding to the desired feature
    RGBPixel_t pixel = image->rgbLines()[y0 + y][x0 + x];
    return (scalar_t) _lookup[pixel.r] + _lookup[pixel.g] * 8 +
                      _lookup[pixel.b] * 64;
}
