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


/** @file   goalplanning_simulator.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'GoalPlanningSimulator' class
*/

#include "goalplanning_simulator.h"
#include "logics/logic_2d_movements.h"
#include "logics/logic_recorded.h"
#include <mash-utils/stringutils.h>
#include <sstream>
#include <iostream>
#include <sys/stat.h> 
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


using namespace std;
using namespace Mash;


/***************************** STATIC ATTRIBUTES ******************************/

GoalPlanningSimulator::tTaskList GoalPlanningSimulator::TASKS;


/************************* CONSTRUCTION / DESTRUCTION *************************/

GoalPlanningSimulator::GoalPlanningSimulator()
: _logic(0)
{
}


GoalPlanningSimulator::~GoalPlanningSimulator()
{
    delete _logic;
}


/******************************* STATIC METHODS *******************************/

IApplicationServer* GoalPlanningSimulator::create()
{
    return new GoalPlanningSimulator();
}


/********************************** METHODS ***********************************/

void GoalPlanningSimulator::setGlobalSeed(unsigned int seed)
{
    _generator.setSeed(seed);
}


tStringList GoalPlanningSimulator::getGoals()
{
    tStringList goals;
    tTaskIterator iter, iterEnd;
    
    for (iter = TASKS.begin(), iterEnd = TASKS.end(); iter != iterEnd; ++iter)
        goals.push_back(iter->first);

    return goals;
}


tStringList GoalPlanningSimulator::getEnvironments(const std::string& goal)
{
    tStringList environments;

    tTaskIterator iter = TASKS.find(goal);
    if (iter != GoalPlanningSimulator::TASKS.end())
    {
        tEnvironmentIterator iter2, iterEnd2;
        for (iter2 = iter->second.begin(), iterEnd2 = iter->second.end();
             iter2 != iterEnd2; ++iter2)
        {
            environments.push_back(iter2->name);
        }
    }

    return environments;
}


tStringList GoalPlanningSimulator::getActions(const std::string& goal,
                                              const std::string& environment)
{
    tStringList actions;

    tEnvironment env;
    if (getEnvironment(goal, environment, &env))
    {
        ILogic* pLogic = 0;

        if (env.recording)
            pLogic = new LogicRecorded(env.url, false);
        else
            pLogic = new Logic2DMovements();

        actions = pLogic->getActions();
        delete pLogic;
    }

    return actions;
}


IApplicationServer::tViewsList GoalPlanningSimulator::getViews(const std::string& goal,
                                                               const std::string& environment)
{
    tViewsList views;

    tEnvironment env;
    if (getEnvironment(goal, environment, &env))
    {
        ILogic* pLogic = 0;

        if (env.recording)
            pLogic = new LogicRecorded(env.url, false);
        else
            pLogic = new Logic2DMovements();

        views = pLogic->getViews();
        delete pLogic;
    }

    return views;
}


tGoalPlanningMode GoalPlanningSimulator::getMode(const std::string& goal,
                                                 const std::string& environment)
{
    tGoalPlanningMode mode = GPMODE_STANDARD;
    
    tEnvironment env;
    if (getEnvironment(goal, environment, &env))
    {
        if (env.recording)
        {
            LogicRecorded* pLogic = new LogicRecorded(env.url, false);
            mode = pLogic->getMode();
            delete pLogic;
        }
    }

    return mode;
}


bool GoalPlanningSimulator::initializeTask(const std::string& goal,
                                           const std::string& environment,
                                           const IApplicationServer::tSettingsList& settings)
{
    // Cleanup the previous world, if any
    if (_logic)
    {
        delete _logic;
        _logic = 0;
    }

    if (getEnvironment(goal, environment, &_environment))
    {
        if (!_environment.recording)
        {
            Logic2DMovements* pLogic = new Logic2DMovements();
            pLogic->setSize(12, 12);
        
            if (!StringUtils::endsWith(goal, "-randomized", false))
                pLogic->setStart(Logic2DMovements::tPosition(0, 0));
        
            pLogic->setGoalOrObstacle(Logic2DMovements::tPosition(9,  4), StringUtils::startsWith(goal, "ReachBlueCylinder", false));
            pLogic->setGoalOrObstacle(Logic2DMovements::tPosition(2,  2), StringUtils::startsWith(goal, "ReachYellowCylinder", false));
            pLogic->setGoalOrObstacle(Logic2DMovements::tPosition(0, 11), StringUtils::startsWith(goal, "ReachGreenCylinder", false));
            pLogic->setGoalOrObstacle(Logic2DMovements::tPosition(7,  7), StringUtils::startsWith(goal, "ReachYellowCube", false));
            pLogic->setGoalOrObstacle(Logic2DMovements::tPosition(9,  9), StringUtils::startsWith(goal, "ReachRedCube", false));

            _logic = pLogic;
        }
        else
        {
            _logic = new LogicRecorded(_environment.url);
        }
    }
    
    if (_logic)
        _logic->reset(&_generator);

    return (_logic != 0);
}


bool GoalPlanningSimulator::resetTask()
{
    _logic->reset(&_generator);

    return true;
}


unsigned int GoalPlanningSimulator::getNbTrajectories()
{
    return _logic->getNbTrajectories();
}


unsigned int GoalPlanningSimulator::getTrajectoryLength(unsigned int trajectory)
{
    if (trajectory >= _logic->getNbTrajectories())
        return 0;

    return _logic->getTrajectoryLength(trajectory);
}


unsigned char* GoalPlanningSimulator::getView(const std::string& viewName, size_t &nbBytes,
                                              std::string &mimetype)
{
    string strFileName = _environment.url + _logic->getViewFileName(viewName);

    mimetype = "image/png";
    nbBytes = 0;
    
    FILE* pFile = fopen(strFileName.c_str(), "rb");
    if (!pFile)
        return 0;

    fseek(pFile, 0, SEEK_END);
    nbBytes = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    
    unsigned char* pBuffer = new unsigned char[nbBytes];
    
    fread(pBuffer, sizeof(unsigned char), nbBytes, pFile);
    
    fclose(pFile);
    
    return pBuffer;
}


bool GoalPlanningSimulator::performAction(const std::string& action, float &reward,
                                          bool &finished, bool &failed,
                                          std::string &event)
{
    _logic->performAction(action, reward, finished, failed);
    
    // No event
    event = "";
    
    return true;
}


string GoalPlanningSimulator::getSuggestedAction()
{
    return _logic->getSuggestedAction();
}


bool GoalPlanningSimulator::getEnvironment(const std::string& goal,
                                           const std::string& environment,
                                           tEnvironment* pEnvironment)
{
    tTaskIterator iter = TASKS.find(goal);
    if (iter != GoalPlanningSimulator::TASKS.end())
    {
        tEnvironmentIterator iter2, iterEnd2;
        for (iter2 = iter->second.begin(), iterEnd2 = iter->second.end();
             iter2 != iterEnd2; ++iter2)
        {
            if (iter2->name == environment)
            {
                *pEnvironment = *iter2;
                return true;
            }
        }
    }
    
    return false;
}
