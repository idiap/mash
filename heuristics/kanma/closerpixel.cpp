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

    This heuristic looks at 9x9 blocks of grayscale pixels an returns the index
    of the one which is the closer (intensity-wise) of the center one.
    
    The indices are:
    0 1 2
    7 C 3
    6 5 4
*/

#include <mash/heuristic.h>

using namespace Mash;


//------------------------------------------------------------------------------
// Declaration of the heuristic class
//------------------------------------------------------------------------------
class CloserPixelHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    CloserPixelHeuristic();
    virtual ~CloserPixelHeuristic();


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim();

    virtual scalar_t computeFeature(unsigned int feature_index);
};


//------------------------------------------------------------------------------
// Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new CloserPixelHeuristic();
}


/************************* CONSTRUCTION / DESTRUCTION *************************/

CloserPixelHeuristic::CloserPixelHeuristic()
{
}


CloserPixelHeuristic::~CloserPixelHeuristic()
{
}


/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int CloserPixelHeuristic::dim()
{
    // We have a margin of 1 pixel on each border of the region of interest
    unsigned int roi_size = roi_extent * 2 + 1;
    return (roi_size - 2) * (roi_size - 2);
}


scalar_t CloserPixelHeuristic::computeFeature(unsigned int feature_index)
{
    // Compute the coordinates of the top-left pixel of the reduced region of
    // interest
    unsigned int x0 = coordinates.x - roi_extent + 1;
    unsigned int y0 = coordinates.y - roi_extent + 1;

    // Compute the coordinates of the pixel corresponding to the feature, in
    // the region of interest
    unsigned int roi_size = ((roi_extent - 1) * 2 + 1);
    unsigned int x = feature_index % roi_size;
    unsigned int y = (feature_index - x) / roi_size;

    // Return the index of the closer surrounding pixel
    const int offsets[][2] = {
        {-1, -1},
        {0, -1},
        {1, -1},
        {0, 1},
        {1, 1},
        {0, 1},
        {-1, 1},
        {-1, 0},
    };
    
    const unsigned int nbOffsets = sizeof(offsets) / (2 * sizeof(int));

    int minSquaredDist = 0;
    unsigned int result = 0;
    byte_t** pLines = image->grayLines();
    byte_t center = pLines[y0 + y][x0 + x];
    for (unsigned int i = 0; i < nbOffsets; ++i)
    {
        byte_t pixel = pLines[y0 + y + offsets[i][0]][x0 + x + offsets[i][1]];
        int squaredDist = ((int) pixel - (int) center) * ((int) pixel - (int) center);
        
        if ((squaredDist < minSquaredDist) || (i == 0))
        {
            minSquaredDist = squaredDist;
            result = i;
        }
    }

    return (scalar_t) result;
}
