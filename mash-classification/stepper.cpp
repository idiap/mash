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


/** @file   stepper.cpp
    @author Charles Dubout (charles.dubout@idiap.ch)

    Implementation of the 'Stepper' class
*/

#include "stepper.h"
#include <stdlib.h>
#include <assert.h>

using namespace std;
using namespace Mash;

/************************* CONSTRUCTION / DESTRUCTION *************************/

Stepper::Stepper(bool detection) : _detection(detection) {}

/********************************* METHODS ************************************/

void Stepper::setParameters(unsigned int roi_extent, unsigned int step_x,
                            unsigned int step_y, float step_z)
{
    _roi_extent = roi_extent;
    _step_x = step_x;
    _step_y = step_y;
    _step_z = step_z;
}

void Stepper::getScales(dim_t image_size, tScalesList* list) const
{
    unsigned int roi_size = _roi_extent * 2 + 1;

    assert(list != 0);

    list->clear();

    if(!_detection)
    {
        list->resize(1);
        list->front() = (float) roi_size / max(image_size.width, image_size.height);
        return;
    }

    float scale = 1.0f;

    do
    {
        list->push_back(scale);
        scale *= _step_z;
    }
    while(image_size.width * scale >= roi_size || image_size.height * scale >= roi_size);
}

void Stepper::getPositions(dim_t image_size, tCoordinatesList* list) const
{
    assert(list != 0);

    list->clear();

    if(!_detection)
    {
        assert(image_size.width == _roi_extent * 2 + 1);
        assert(image_size.height == _roi_extent * 2 + 1);
        list->resize(1);
        list->front().x = _roi_extent;
        list->front().y = _roi_extent;
        return;
    }

    // Find the first pair of coordinates (top-left corner)
    coordinates_t coords;

    // Start from the middle (so that the coordinates are symetric)
    coords.x = (image_size.width  - 1) >> 1;
    coords.y = (image_size.height - 1) >> 1;

    // Determine the number of steps to do to go from the middle to the top-left corner
    unsigned int nb_steps_x = (coords.x - _roi_extent) / _step_x;
    unsigned int nb_steps_y = (coords.y - _roi_extent) / _step_y;

    // Go to the top-left corner
    coords.x -= nb_steps_x * _step_x;
    coords.y -= nb_steps_y * _step_y;

    // Iterates the coordinates on the grid, alternating between left to right
    // and right to left
    for(unsigned int y = 0; y <= nb_steps_y << 1; ++y)
    {
        if(y & 1)
        {
            for(unsigned int x = 0; x <= nb_steps_x << 1; ++x)
            {
                coords.x -= _step_x;
                list->push_back(coords);
            }
        }
        else
        {
            for(unsigned int x = 0; x <= nb_steps_x << 1; ++x)
            {
                list->push_back(coords);
                coords.x += _step_x;
            }
        }

        coords.y += _step_y;
    }
}

coordinates_t Stepper::getClosestPosition(dim_t image_size, coordinates_t position) const
{
    // Find the closest pair of coordinates
    coordinates_t coords;

    // Start from the middle (so that the coordinates are symetric)
    coords.x = (image_size.width  - 1) >> 1;
    coords.y = (image_size.height - 1) >> 1;

    if(!_detection)
    {
        return coords;
    }

    // Determine the number of steps to do to go from the middle to the top-left corner
    unsigned int nb_steps_x = (coords.x - _roi_extent) / _step_x;
    unsigned int nb_steps_y = (coords.y - _roi_extent) / _step_y;

    // Go to the top-left corner
    coords.x -= nb_steps_x * _step_x;
    coords.y -= nb_steps_y * _step_y;

    coordinates_t closest;

    int smallest_dist_x = -1;

    for(unsigned int x = 0; x <= nb_steps_x << 1; ++x)
    {
        int dist = abs(int(position.x) - int(coords.x));

        if(dist < smallest_dist_x || smallest_dist_x < 0)
        {
            closest.x = coords.x;
            smallest_dist_x = dist;
        }

        coords.x += _step_x;
    }

    int smallest_dist_y = -1;

    for(unsigned int y = 0; y <= nb_steps_y << 1; ++y)
    {
        int dist = abs(int(position.y) - int(coords.y));

        if(dist < smallest_dist_y || smallest_dist_y < 0)
        {
            closest.y = coords.y;
            smallest_dist_y = dist;
        }

        coords.y += _step_y;
    }

    return closest;
}
