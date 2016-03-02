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


/** @file   logic_2d_movements.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'Logic2DMovements' class
*/

#include "logic_2d_movements.h"
#include <sstream>


using namespace std;
using namespace Mash;


const float REWARD_GOAL     = 100.0f;
const float REWARD_OBSTACLE = -10.0f;
const float REWARD_BORDER   = -5.0f;
const float REWARD_STEP     = -1.0f;


/************************* CONSTRUCTION / DESTRUCTION *************************/

Logic2DMovements::Logic2DMovements()
: _width(-1), _height(-1), _position(-1, -1), _goal(-1, -1), _start(-1, -1)
{
}


Logic2DMovements::~Logic2DMovements()
{
}


/********************************** METHODS ***********************************/

tStringList Logic2DMovements::getActions()
{
    tStringList actions;
    actions.push_back("GO_NORTH");
    actions.push_back("GO_SOUTH");
    actions.push_back("GO_WEST");
    actions.push_back("GO_EAST");
    
    return actions;
}


IApplicationServer::tViewsList Logic2DMovements::getViews()
{
    IApplicationServer::tViewsList views;
    IApplicationServer::tView view;

    view.name = "gri";
    view.width = 1280;
    view.height = 960;
    views.push_back(view);

    view.name = "obl";
    view.width = 1280;
    view.height = 960;
    views.push_back(view);

    view.name = "top";
    view.width = 1280;
    view.height = 960;
    views.push_back(view);

    return views;
}


void Logic2DMovements::reset(Mash::RandomNumberGenerator* generator)
{
    if ((_start.x >= 0) && (_start.y >= 0))
    {
        _position = _start;
        return;
    }
    
    _position.x = -1;

    while (_position.x < 0)
    {
        _position.x = generator->randomize(_width - 1);
        _position.y = generator->randomize(_height - 1);

        if (((_goal.x == _position.x) && (_goal.y == _position.y)) || obstacleAtPosition(_position))
            _position.x = -1;
    }
}


const std::string Logic2DMovements::getViewFileName(const std::string& viewName)
{
    ostringstream str;
    
    str << viewName << "/exp22_pos_";
    
    if (_position.x >= 10)
        str << _position.x;
    else
        str << "0" << _position.x;

    str << "_";

    if (_position.y >= 10)
        str << _position.y;
    else
        str << "0" << _position.y;

    str << "_" << viewName << ".png";
    
    return str.str();
}


void Logic2DMovements::performAction(const std::string& action, float &reward,
                                     bool &finished, bool &failed)
{
    finished = false;
    failed = false;
    reward = REWARD_STEP;
    
    tPosition previous = _position;
    
    if (action == "GO_NORTH")
        ++_position.y;
    else if (action == "GO_SOUTH")
        --_position.y;
    else if (action == "GO_EAST")
        ++_position.x;
    else if (action == "GO_WEST")
        --_position.x;

    if ((_position.y >= _height) || (_position.y < 0) || (_position.x >= _width) || (_position.x < 0))
    {
        reward = REWARD_BORDER;
        _position = previous;
    }
    else if (obstacleAtPosition(_position))
    {
        reward = REWARD_OBSTACLE;
        _position = previous;
    }
    else if ((_position.x == _goal.x) && (_position.y == _goal.y))
    {
        finished = true;
        reward = REWARD_GOAL;
    }
}


bool Logic2DMovements::obstacleAtPosition(const tPosition& pos)
{
    tPositionIterator iter, iterEnd;
    for (iter = _obstacles.begin(), iterEnd = _obstacles.end(); iter != iterEnd; ++iter)
    {
        if ((iter->x == pos.x) && (iter->y == pos.y))
            return true;
    }
    
    return false;
}
