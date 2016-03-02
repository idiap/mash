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


/** @file   world.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'World' class
*/

#include "world.h"
#include <assert.h>


using namespace std;


/************************* CONSTRUCTION / DESTRUCTION *************************/

World::World(int width, int height)
: _width(width), _height(height)
{
    assert(width > 0);
    assert(height > 0);

    _start.x    = 0;
    _start.y    = 0;
    _end.x      = 0;
    _end.y      = 0;
    _position.x = 0;
    _position.y = 0;
}


World::~World()
{
}


/********************** IMPLEMENTATION OF ServerListener **********************/

bool World::performAction(tAction action, float &reward)
{
    reward = 0.0f;
    _strEvent = "";

    switch (action)
    {
        case ACTION_GO_NORTH:
            _position.y--;
            if (_position.y < 0)
            {
                _position.y = 0;
                reward = -1.0f;
                _strEvent = "Hit north wall";
            }
            break;

        case ACTION_GO_SOUTH:
            _position.y++;
            if (_position.y >= _height)
            {
                _position.y = _height - 1;
                reward = -1.0f;
                _strEvent = "Hit south wall";
            }
            break;

        case ACTION_GO_EAST:
            _position.x++;
            if (_position.x >= _width)
            {
                _position.x = _width - 1;
                reward = -1.0f;
                _strEvent = "Hit east wall";
            }
            break;

        case ACTION_GO_WEST:
            _position.x--;
            if (_position.x < 0)
            {
                _position.x = 0;
                reward = -1.0f;
                _strEvent = "Hit west wall";
            }
            break;
    }

    if ((_position.x == _end.x) && (_position.y == _end.y))
    {
        _strEvent = "Goal reached";
        reward = 10.0f;
        return true;
    }
    
    return false;
}


unsigned char* World::draw(int width, int height)
{
    // Assertions
    assert(width > 0);
    assert(height > 0);
    
    // Declarations
    unsigned char* pImage;
    unsigned int total_size;
    int deltax, deltay, delta, x0, y0;
    
    // Allocate the buffer
    total_size = 3 * width * height;
    pImage = new unsigned char[total_size];
    
    // Initializations
    deltax = width / _width;
    deltay = height / _height;

    delta = (deltax <= deltay) ? deltax : deltay;

    x0 = (width - delta * _width) / 2;
    y0 = (height - delta * _height) / 2;

    // Pixels
    unsigned char* pDst = pImage;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int xx = (x - x0) / delta;
            int dx = (x - x0) % delta;
            int yy = (y - y0) / delta;
            int dy = (y - y0) % delta;
            
            // Inside a cell?
            if ((dx != 0) && (dx != delta - 1) && (dy != 0) && (dy != delta - 1) &&
                (x >= x0) && (xx < _width) && (y >= y0) && (yy < _height))
            {
                // Agent location
                if ((xx == _position.x) && (yy == _position.y))
                {
                    pDst[0] = 255;
                    pDst[1] = 0;
                    pDst[2] = 0;
                }
                
                // Goal location
                else if ((xx == _end.x) && (yy == _end.y))
                {
                    pDst[0] = 0;
                    pDst[1] = 0;
                    pDst[2] = 255;
                }

                // Elsewhere
                else
                {
                    pDst[0] = 0;
                    pDst[1] = 255;
                    pDst[2] = 0;
                }
            }
            else
            {
                pDst[0] = 0;
                pDst[1] = 0;
                pDst[2] = 0;
            }
            
            pDst += 3;
        }
    }

    return pImage;
}
