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


/** @file   logic_recorded.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'LogicRecorded' class
*/

#include "logic_recorded.h"
#include <mash-network/commands_serializer.h>
#include <mash-utils/stringutils.h>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

LogicRecorded::LogicRecorded(const std::string& strDatabasePath, bool bLoadAll)
: _mode(GPMODES_COUNT), _currentTrajectory(0), _currentStep(0), _firstResetDone(false)
{
    string strIndexFile;
    CommandsSerializer serializer;
        
    if (strDatabasePath.at(strDatabasePath.length() - 1) == '/')
        strIndexFile = strDatabasePath + "index.txt";
    else
        strIndexFile = strDatabasePath + "/index.txt";
    
    if (serializer.deserialize(strIndexFile))
    {
        CommandsSerializer::tCommand command = serializer.getCommand(0);

        if (command.strCommand == "RECORDED_TEACHER")
            _mode = GPMODE_RECORDED_TEACHER;
        else if (command.strCommand == "RECORDED_TRAJECTORIES")
            _mode = GPMODE_RECORDED_TRAJECTORIES;

        command = serializer.getCommand(1);
        if (command.strCommand == "AVAILABLE_ACTIONS")
        {
            for (unsigned int j = 0; j < command.arguments.size(); ++j)
                _actions.push_back(command.arguments.getString(j));
        }

        command = serializer.getCommand(2);
        if (command.strCommand == "AVAILABLE_VIEWS")
        {
            for (unsigned int i = 0; i < command.arguments.size(); ++i)
            {
                tStringList parts = StringUtils::split(command.arguments.getString(i), ":x");

                IApplicationServer::tView view;

                view.name = parts[0];
                view.width = StringUtils::parseUnsignedInt(parts[1]);
                view.height = StringUtils::parseUnsignedInt(parts[2]);
                
                _views.push_back(view);
            }
        }

        if (bLoadAll)
        {
            tTrajectory trajectory;
            tStep step;

            for (unsigned int i = 3; i < serializer.nbCommands(); ++i)
            {
                command = serializer.getCommand(i);

                if (command.strCommand == "TRAJECTORY_START")
                {
                    if (!step.images.empty())
                    {
                        trajectory.steps.push_back(step);
                    
                        step.images.clear();
                        step.action = "";
                        step.reward = 0.0f;
                    }

                    if (!trajectory.steps.empty())
                        _trajectories.push_back(trajectory);
                
                    trajectory.steps.clear();
                    trajectory.finished = false;
                    trajectory.failed = false;
                }
                else if (command.strCommand == "IMAGES")
                {
                    if (!step.images.empty() && !trajectory.finished && !trajectory.failed)
                    {
                        trajectory.steps.push_back(step);
                    
                        step.images.clear();
                        step.action = "";
                        step.reward = 0.0f;
                    }

                    for (unsigned int j = 0; j < command.arguments.size(); ++j)
                        step.images.push_back(command.arguments.getString(j));
                }
                else if (command.strCommand == "ACTION")
                {
                    step.action = command.arguments.getString(0);
                }
                else if (command.strCommand == "REWARD")
                {
                    step.reward = command.arguments.getFloat(0);
                }
                else if (command.strCommand == "FINISHED")
                {
                    trajectory.finished = true;
                }
                else if (command.strCommand == "FAILED")
                {
                    trajectory.failed = true;
                }
            }

            if (!step.images.empty())
                trajectory.steps.push_back(step);

            if (!trajectory.steps.empty())
                _trajectories.push_back(trajectory);
        }
    }
}


LogicRecorded::~LogicRecorded()
{
}


/********************************** METHODS ***********************************/

void LogicRecorded::reset(Mash::RandomNumberGenerator* generator)
{
    if (_firstResetDone)
    {
        _currentStep = 0;
    
        ++_currentTrajectory;
        if (_currentTrajectory >= getNbTrajectories())
            _currentTrajectory = 0;
    }
    
    _firstResetDone = true;
}


const std::string LogicRecorded::getViewFileName(const std::string& viewName)
{
    IApplicationServer::tViewsList::iterator iter, iterEnd;
    unsigned int index;
    
    for (index = 0, iter = _views.begin(), iterEnd = _views.end();
         iter != iterEnd; ++iter, ++index)
    {
        if (iter->name == viewName)
            return _trajectories[_currentTrajectory].steps[_currentStep].images[index];
    }
    
    return "";
}


void LogicRecorded::performAction(const std::string& action, float &reward,
                                  bool &finished, bool &failed)
{
    finished = false;
    failed = false;
    reward = 0.0f;

    if ((action == _trajectories[_currentTrajectory].steps[_currentStep].action) &&
        (_currentStep < _trajectories[_currentTrajectory].steps.size() - 1))
    {
        ++_currentStep;
        
        reward = _trajectories[_currentTrajectory].steps[_currentStep - 1].reward;

        if (_currentStep == _trajectories[_currentTrajectory].steps.size() - 1)
        {
            finished = _trajectories[_currentTrajectory].finished;
            failed = _trajectories[_currentTrajectory].failed;
        }
    }
}
