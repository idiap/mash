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


/** @file   mean_threshold.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Example of a heuristic: MeanThreshold. The features indicates if the
    corresponding (grayscale) pixels is above or below the mean value of the
    processed region of interest.
*/

#include <mash/heuristic.h>

using namespace Mash;


//------------------------------------------------------------------------------
/// @brief  The 'MeanThreshold' heuristic class
//------------------------------------------------------------------------------
class MeanThresholdHeuristic: public Heuristic
{
    //_____ Construction / Destruction __________
public:
    MeanThresholdHeuristic();
    virtual ~MeanThresholdHeuristic();


    //_____ Implementation of Heuristic __________
public:
    virtual unsigned int dim();

    virtual void prepareForCoordinates();

    virtual scalar_t computeFeature(unsigned int feature_index);


    //_____ Attributes __________
protected:
    float _mean;
};


//------------------------------------------------------------------------------
/// @brief  Creation function of the heuristic
//------------------------------------------------------------------------------
extern "C" Heuristic* new_heuristic()
{
    return new MeanThresholdHeuristic();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

MeanThresholdHeuristic::MeanThresholdHeuristic()
: _mean(0.0f)
{
}


MeanThresholdHeuristic::~MeanThresholdHeuristic()
{
}


/************************* IMPLEMENTATION OF Heuristic ************************/

unsigned int MeanThresholdHeuristic::dim()
{
    // We have has many features than pixels in the region of interest
    unsigned int roi_size = roi_extent * 2 + 1;
    return roi_size * roi_size;
}


void MeanThresholdHeuristic::prepareForCoordinates()
{
    // Compute the coordinates of the top-left pixel of the region of interest
    unsigned int x0 = coordinates.x - roi_extent;
    unsigned int y0 = coordinates.y - roi_extent;

    // Compute the mean value of the region of interest
    byte_t** pLines = image->grayLines();
    
    _mean = 0.0f;
    for (unsigned int y = 0; y < roi_extent * 2 + 1; ++y)
    {
        for (unsigned int x = 0; x < roi_extent * 2 + 1; ++x)
            _mean += pLines[y0 + y][x0 + x];
    }

    _mean /= dim();
}


scalar_t MeanThresholdHeuristic::computeFeature(unsigned int feature_index)
{
    // Compute the coordinates of the top-left pixel of the region of interest
    unsigned int x0 = coordinates.x - roi_extent;
    unsigned int y0 = coordinates.y - roi_extent;

    // Compute the coordinates of the pixel corresponding to the feature, in
    // the region of interest
    unsigned int roi_size = roi_extent * 2 + 1;
    unsigned int x = feature_index % roi_size;
    unsigned int y = (feature_index - x) / roi_size;

    // Compute the feature value
    byte_t** pLines = image->grayLines();
    if (pLines[y0 + y][x0 + x] >= _mean)
        return 1.0f;
        
    return 0.0f;
}
