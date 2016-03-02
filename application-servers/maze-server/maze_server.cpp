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


/** @file   maze_server.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'MazeServer' class
*/

#include "maze_server.h"
#include <sstream>
#include <iostream>
#include <sys/stat.h> 
#include <stdlib.h>
#include <assert.h>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

MazeServer::MazeServer()
: _world(0)
{
}


MazeServer::~MazeServer()
{
    delete _world;
}


/******************************* STATIC METHODS *******************************/

IApplicationServer* MazeServer::create()
{
    return new MazeServer();
}


/********************************** METHODS ***********************************/

void MazeServer::setGlobalSeed(unsigned int seed)
{
    RandomNumberGenerator globalGenerator;
    
    globalGenerator.setSeed(seed);
    
    for (unsigned int i = 0; i < COUNT_RANDOM_NUMBER_GENERATORS; ++i)
        _randomGenerators[i].setSeed(globalGenerator.randomize());
}


tStringList MazeServer::getGoals()
{
    tStringList goals;
    
    goals.push_back("ReachBlueCell");
    goals.push_back("ReachBlueCell-randomized");

    return goals;
}


tStringList MazeServer::getEnvironments(const std::string& goal)
{
    tStringList environments;
    
    environments.push_back("static4x4");
    environments.push_back("static6x4");
    environments.push_back("static6x6");
    environments.push_back("static10x10");
    environments.push_back("dynamic");

    return environments;
}


tStringList MazeServer::getActions(const std::string& goal,
                                   const std::string& environment)
{
    tStringList actions;

    // We have the same actions for all the tasks
    actions.push_back("GO_NORTH");
    actions.push_back("GO_SOUTH");
    actions.push_back("GO_EAST");
    actions.push_back("GO_WEST");

    return actions;
}


IApplicationServer::tViewsList MazeServer::getViews(const std::string& goal,
                                                    const std::string& environment)
{
    tViewsList views;

    tView view;
    view.name = "main";
    view.width = 320;
    view.height = 320;

    views.push_back(view);

    return views;
}


tGoalPlanningMode MazeServer::getMode(const std::string& goal,
                                      const std::string& environment)
{
    return GPMODE_STANDARD;
}


bool MazeServer::initializeTask(const std::string& goal,
                                const std::string& environment,
                                const IApplicationServer::tSettingsList& settings)
{
    // Cleanup the previous world, if any
    if (_world)
        delete _world;

    // Initialize the world
    if (environment == "static4x4")
        _world = new World(4, 4);
    else if (environment == "static6x4")
        _world = new World(6, 4);
    else if (environment == "static6x6")
        _world = new World(6, 6);
    else if (environment == "static10x10")
        _world = new World(10, 10);
    else if (environment == "dynamic")
        _world = new World(_randomGenerators[RND_ENVIRONMENT].randomize(4, 10),
                           _randomGenerators[RND_ENVIRONMENT].randomize(4, 10));
    
    if (goal == "ReachBlueCell-randomized")
    {
        int start_x = _randomGenerators[RND_GOAL].randomize(_world->getWidth() - 1);
        int start_y = _randomGenerators[RND_GOAL].randomize(_world->getHeight() - 1);

        int end_x = start_x;
        int end_y = start_y;

        while ((end_x == start_x) && (end_y == start_y))
        {
            end_x = _randomGenerators[RND_GOAL].randomize(_world->getWidth() - 1);
            end_y = _randomGenerators[RND_GOAL].randomize(_world->getHeight() - 1);
        }

        _world->setStart(start_x, start_y);
        _world->setEnd(end_x, end_y);
    }
    else
    {
        _world->setStart(0, 0);
        _world->setEnd(_world->getWidth() - 1, _world->getHeight() - 1);
    }
    
    _world->reset();

    return true;
}


bool MazeServer::resetTask()
{
    // Reset the world
    _world->reset();

    return true;
}


unsigned int MazeServer::getNbTrajectories()
{
    return 0;
}


unsigned int MazeServer::getTrajectoryLength(unsigned int trajectory)
{
    return 0;
}


unsigned char* MazeServer::getView(const std::string& viewName, size_t &nbBytes,
                                   std::string &mimetype)
{
    mimetype = "raw";
    nbBytes = 320 * 320 * 3;
    
    return _world->draw(320, 320);
}


bool MazeServer::performAction(const std::string& action, float &reward,
                               bool &finished, bool &failed, std::string &event)
{
    tAction theAction;
    
    if (action == "GO_NORTH")
        theAction = ACTION_GO_NORTH;
    else if (action == "GO_SOUTH")
        theAction = ACTION_GO_SOUTH;
    else if (action == "GO_EAST")
        theAction = ACTION_GO_EAST;
    else if (action == "GO_WEST")
        theAction = ACTION_GO_WEST;
    
    // Perform the action
    finished = _world->performAction(theAction, reward);

    // Retrieve the event (if any)
    event = _world->event();
    
    // Can't fail
    failed = false;
    
    return true;
}
