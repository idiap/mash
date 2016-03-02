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


/** @file   sandbox_task.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxTask' class
*/

#include "sandbox_task.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <iostream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxTask::SandboxTask(const CommunicationChannel& channel,
                         OutStream* pOutStream,
                         tWardenContext* pWardenContext,
                         bool bReadOnly)
: _channel(channel), _perception(channel, pOutStream, pWardenContext, bReadOnly),
  _pWardenContext(pWardenContext), _bReadOnly(bReadOnly), _mode(GPMODES_COUNT),
  _nbActions(0), _result(RESULT_NONE), _suggestedAction(-1)
{
    if (pOutStream)
        _outStream = *pOutStream;
}


SandboxTask::~SandboxTask()
{
}


/*********************************** METHODS **********************************/

tGoalPlanningMode SandboxTask::mode()
{
    if (_mode == GPMODES_COUNT)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);
 
        _outStream << "< TASK_MODE" << endl;

        // Send the command to the task
        _channel.startPacket(SANDBOX_COMMAND_TASK_MODE);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the mode
        if (result)
            _channel.read((unsigned int*) &_mode);

        setWardenContext(pPreviousContext);
    }
    
    return _mode;
}


unsigned int SandboxTask::nbActions()
{
    if (_nbActions == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);
 
        _outStream << "< TASK_NB_ACTIONS" << endl;

        // Send the command to the task
        _channel.startPacket(SANDBOX_COMMAND_TASK_NB_ACTIONS);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the number of actions
        if (result)
            _channel.read(&_nbActions);

        setWardenContext(pPreviousContext);
    }
    
    return _nbActions;
}


unsigned int SandboxTask::nbTrajectories()
{
    if (_trajectoryLengths.empty())
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);
 
        _outStream << "< TASK_NB_TRAJECTORIES" << endl;

        // Send the command to the task
        _channel.startPacket(SANDBOX_COMMAND_TASK_NB_TRAJECTORIES);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the number of trajectories
        unsigned int nbTrajectories = 0;
        if (result && _channel.read(&nbTrajectories))
        {
            for (unsigned int i = 0; i < nbTrajectories; ++i)
                _trajectoryLengths.push_back(0);
        }

        setWardenContext(pPreviousContext);
    }
    
    return _trajectoryLengths.size();
}


unsigned int SandboxTask::trajectoryLength(unsigned int trajectory)
{
    if (_trajectoryLengths.empty() && (nbTrajectories() == 0))
        return 0;

    if (trajectory >= _trajectoryLengths.size())
        return 0;

    if (_trajectoryLengths[trajectory] == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< TASK_TRAJECTORY_LENGTH " << trajectory << endl;
        
        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_TASK_TRAJECTORY_LENGTH);
        _channel.add(trajectory);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the length of the trajectory
        unsigned int length = 0;
        if (result && _channel.read(&length))
            _trajectoryLengths[trajectory] = length;

        setWardenContext(pPreviousContext);
    }
    
    return _trajectoryLengths[trajectory];
}


bool SandboxTask::reset()
{
    if (_bReadOnly)
        return false;

    tWardenContext* pPreviousContext = getWardenContext();
    setWardenContext(0);
    
    _outStream << "< TASK_RESET " << endl;
        
    // Send the command to the task
    _channel.startPacket(SANDBOX_COMMAND_TASK_RESET);
    _channel.sendPacket();

    // Wait the response
    bool result = _channel.good();
    if (result)
        result = waitResponse();
    
    _result = RESULT_NONE;
    
    setWardenContext(pPreviousContext);
    
    _suggestedAction = -1;
    
    return result;
}


bool SandboxTask::performAction(unsigned int action, scalar_t* reward)
{
    if (_bReadOnly)
        return 0.0f;

    if (action >= nbActions())
        return 0.0f;

    tWardenContext* pPreviousContext = getWardenContext();
    setWardenContext(0);

    _outStream << "< TASK_PERFORM_ACTION " << action << endl;
        
    // Send the command to the task
    _channel.startPacket(SANDBOX_COMMAND_TASK_PERFORM_ACTION);
    _channel.add(action);
    _channel.sendPacket();

    // Wait the response
    bool result = _channel.good();
    if (result)
        result = waitResponse();
    
    // Retrieve the reward and the result
    *reward = 0.0f;
    unsigned int actionResult = RESULT_NONE;
    _suggestedAction = 0;
    
    if (result)
    {
        _channel.read(reward);
        _channel.read(&actionResult);

        if (mode() != GPMODE_STANDARD)
            _channel.read(&_suggestedAction);
    }
    
    _result = (tResult) actionResult;
    
    setWardenContext(pPreviousContext);
    
    return result && _channel.good();
}


unsigned int SandboxTask::suggestedAction()
{
    if (_suggestedAction < 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);
 
        _outStream << "< TASK_SUGGESTED_ACTION" << endl;

        // Send the command to the task
        _channel.startPacket(SANDBOX_COMMAND_TASK_SUGGESTED_ACTION);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the suggested action
        if (result)
        {
            unsigned int v;
            _channel.read(&v);
            _suggestedAction = (int) v;
        }
        
        setWardenContext(pPreviousContext);
    }
    
    return (unsigned) _suggestedAction;
}


/********************** COMMUNICATION_RELATED METHODS *************************/

bool SandboxTask::waitResponse()
{
    // Declarations
    tSandboxMessage message;
    
    while (_channel.good())
    {
        if (_channel.receivePacket(&message) != ERROR_NONE)
            break;
        
        if (message == SANDBOX_MESSAGE_RESPONSE)
            return true;

        if (message == SANDBOX_COMMAND_TERMINATE)
        {
            _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
            _channel.sendPacket();
            _exit(0);
        }

        if (message == SANDBOX_MESSAGE_ERROR)
        {
            tError error;
            _channel.read(&error);
            _outStream << "> ERROR " << getErrorDescription(error) << endl;
            break;
        }

        if (message != SANDBOX_MESSAGE_KEEP_ALIVE)
        {
            _outStream << getErrorDescription(ERROR_CHANNEL_UNEXPECTED_RESPONSE) << ": " << message << endl;
            break;
        }
    }
    
    // Error handling
    return false;
}
