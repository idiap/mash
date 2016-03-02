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


/** @file   world.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'World' class
*/

#ifndef _WORLD_H_
#define _WORLD_H_

#include <string>


struct tCoordinates
{
    int x;
    int y;
};


enum tAction
{
    ACTION_GO_NORTH,
    ACTION_GO_SOUTH,
    ACTION_GO_EAST,
    ACTION_GO_WEST,
    
    ACTIONS_COUNT
};



class World
{
    //_____ Construction / Destruction __________
public:
    World(int width, int height);
    ~World();


    //_____ Methods __________
public:
    inline void setStart(int x, int y)
    {
        _start.x = x;
        _start.y = y;
    }
    
    inline void setEnd(int x, int y)
    {
        _end.x = x;
        _end.y = y;
    }

    inline void reset()
    {
        _position = _start;
        _strEvent = "";
    }

    inline std::string event() const
    {
        return _strEvent;
    }

    bool performAction(tAction action, float &reward);
    unsigned char* draw(int width, int height);

    inline int getWidth() const
    {
        return _width;
    }

    inline int getHeight() const
    {
        return _height;
    }


    //_____ Attributes __________
private:
    int             _width;
    int             _height;
    tCoordinates    _start;
    tCoordinates    _end;
    tCoordinates    _position;
    std::string     _strEvent;
};

#endif
