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


/** @file   sandbox_task_proxy.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxTaskProxy' class
*/

#include "sandbox_task_proxy.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-goalplanning/task.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <sstream>


using namespace std;
using namespace Mash;
using namespace Mash::SandboxControllerDeclarations;


/****************************** STATIC ATTRIBUTES *****************************/

SandboxTaskProxy::tCommandHandlersList SandboxTaskProxy::handlers;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxTaskProxy::SandboxTaskProxy(ITask* pTask, const CommunicationChannel& channel)
: _channel(channel), _pTask(pTask), _pPerception(0)
{
    if (handlers.empty())
        setupHandlers();
}


SandboxTaskProxy::SandboxTaskProxy(IPerception* pPerception, const CommunicationChannel& channel)
: _channel(channel), _pTask(0), _pPerception(pPerception)
{
    if (handlers.empty())
        setupHandlers();
}


SandboxTaskProxy::~SandboxTaskProxy()
{
}


/**************************** COMMAND HANDLERS ********************************/

tCommandProcessingResult SandboxTaskProxy::processResponse(tSandboxMessage message)
{
    // Assertions
    assert(!handlers.empty());

    if (!_pTask && !_pPerception)
        return COMMAND_UNKNOWN;

    tCommandHandlersIterator iter = handlers.find(message);
    if (iter != handlers.end())
    {
        tCommandHandler handler = iter->second;
        return (this->*handler)();
    }

    return COMMAND_UNKNOWN;
}


void SandboxTaskProxy::setupHandlers()
{
    assert(handlers.empty());

    handlers[SANDBOX_COMMAND_TASK_MODE]                         = &SandboxTaskProxy::handleTaskModeCommand;
    handlers[SANDBOX_COMMAND_TASK_NB_ACTIONS]                   = &SandboxTaskProxy::handleTaskNbActionsCommand;
    handlers[SANDBOX_COMMAND_TASK_NB_TRAJECTORIES]              = &SandboxTaskProxy::handleTaskNbTrajectoriesCommand;
    handlers[SANDBOX_COMMAND_TASK_TRAJECTORY_LENGTH]            = &SandboxTaskProxy::handleTaskTrajectoryLengthCommand;
    handlers[SANDBOX_COMMAND_TASK_RESET]                        = &SandboxTaskProxy::handleTaskResetCommand;
    handlers[SANDBOX_COMMAND_TASK_PERFORM_ACTION]               = &SandboxTaskProxy::handleTaskPerformActionCommand;
    handlers[SANDBOX_COMMAND_TASK_SUGGESTED_ACTION]             = &SandboxTaskProxy::handleTaskSuggestedActionCommand;

    handlers[SANDBOX_COMMAND_PERCEPTION_NB_HEURISTICS]          = &SandboxTaskProxy::handlePerceptionNbHeuristicsCommand;
    handlers[SANDBOX_COMMAND_PERCEPTION_NB_FEATURES]            = &SandboxTaskProxy::handlePerceptionNbFeaturesCommand;
    handlers[SANDBOX_COMMAND_PERCEPTION_HEURISTIC_NAME]         = &SandboxTaskProxy::handlePerceptionHeuristicNameCommand;
    handlers[SANDBOX_COMMAND_PERCEPTION_HEURISTIC_SEED]         = &SandboxTaskProxy::handlePerceptionHeuristicSeedCommand;
    handlers[SANDBOX_COMMAND_PERCEPTION_NB_VIEWS]               = &SandboxTaskProxy::handlePerceptionNbViewsCommand;
    handlers[SANDBOX_COMMAND_PERCEPTION_COMPUTE_SOME_FEATURES]  = &SandboxTaskProxy::handlePerceptionComputeSomeFeaturesCommand;
    handlers[SANDBOX_COMMAND_PERCEPTION_VIEW_SIZE]              = &SandboxTaskProxy::handlePerceptionViewSizeCommand;
    handlers[SANDBOX_COMMAND_PERCEPTION_ROI_EXTENT]             = &SandboxTaskProxy::handlePerceptionRoiExtentCommand;
}


tCommandProcessingResult SandboxTaskProxy::handleTaskModeCommand()
{
    // Assertions
    assert(_pTask);
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pTask->mode());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handleTaskNbActionsCommand()
{
    // Assertions
    assert(_pTask);
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pTask->nbActions());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handleTaskNbTrajectoriesCommand()
{
    // Assertions
    assert(_pTask);
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pTask->nbTrajectories());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handleTaskTrajectoryLengthCommand()
{
    // Assertions
    assert(_pTask);
    
    // Retrieve the trajectory
    unsigned int trajectory;
    if (!_channel.read(&trajectory))
        return SOURCE_PLUGIN_CRASHED;
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pTask->trajectoryLength(trajectory));
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handleTaskResetCommand()
{
    // Assertions
    assert(_pTask);

    if (!_pTask->reset())
        return APPSERVER_ERROR;
        
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handleTaskPerformActionCommand()
{
    // Assertions
    assert(_pTask);

    // Retrieve the action
    unsigned int action;
    if (!_channel.read(&action))
        return SOURCE_PLUGIN_CRASHED;

    scalar_t reward = 0.0f;
    if (_pTask->performAction(action, &reward))
    {
        unsigned int result = (unsigned int) _pTask->result();

        // Send the response
        _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
        _channel.add(reward);
        _channel.add(result);
        
        if (_pTask->mode() != GPMODE_STANDARD)
            _channel.add(_pTask->suggestedAction());
        
        _channel.sendPacket();
    }
    else
    {
        // Send the response
        _channel.startPacket(SANDBOX_MESSAGE_ERROR);
        _channel.add(ERROR_APPSERVER_ERROR);
        _channel.sendPacket();
    }
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handleTaskSuggestedActionCommand()
{
    // Assertions
    assert(_pTask);
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pTask->suggestedAction());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handlePerceptionNbHeuristicsCommand()
{
    // Assertions
    assert(_pTask || _pPerception);
    
    IPerception* pPerception = (_pPerception ? _pPerception : _pTask->perception());
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(pPerception->nbHeuristics());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handlePerceptionNbFeaturesCommand()
{
    // Assertions
    assert(_pTask || _pPerception);
    
    IPerception* pPerception = (_pPerception ? _pPerception : _pTask->perception());
    
    // Retrieve the index of the heuristic
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
        return SOURCE_PLUGIN_CRASHED;

    // Retrieve the number of features
    unsigned int nbFeatures = pPerception->nbFeatures(heuristic);

    if (dynamic_cast<Perception*>(pPerception))
    {
        tError error = dynamic_cast<Perception*>(pPerception)->featuresComputer()->getLastError();
        if (error == ERROR_HEURISTIC_TIMEOUT)
            return DEST_PLUGIN_TIMEOUT;
        else if (error != ERROR_NONE)
            return DEST_PLUGIN_CRASHED;
    }
    else if (nbFeatures == 0)
    {
        return DEST_PLUGIN_CRASHED;
    }

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(nbFeatures);
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handlePerceptionHeuristicNameCommand()
{
    // Assertions
    assert(_pTask || _pPerception);
    
    IPerception* pPerception = (_pPerception ? _pPerception : _pTask->perception());

    // Retrieve the index of the heuristic
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
        return SOURCE_PLUGIN_CRASHED;

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(pPerception->heuristicName(heuristic));
    _channel.sendPacket();

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handlePerceptionHeuristicSeedCommand()
{
    // Assertions
    assert(_pTask || _pPerception);
    
    IPerception* pPerception = (_pPerception ? _pPerception : _pTask->perception());

    // Retrieve the index of the heuristic
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
        return SOURCE_PLUGIN_CRASHED;

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(pPerception->heuristicSeed(heuristic));
    _channel.sendPacket();

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handlePerceptionNbViewsCommand()
{
    // Assertions
    assert(_pTask || _pPerception);
    
    IPerception* pPerception = (_pPerception ? _pPerception : _pTask->perception());
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(pPerception->nbViews());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handlePerceptionComputeSomeFeaturesCommand()
{
    // Assertions
    assert(_pTask || _pPerception);
    
    IPerception* pPerception = (_pPerception ? _pPerception : _pTask->perception());

    // Declarations
    unsigned int    view;
    unsigned int    heuristic;
    coordinates_t   coordinates;
    unsigned int    nbFeatures;
    unsigned int*   indexes = 0;
    scalar_t*       values = 0;

    // Retrieve all the parameters
    _channel.read(&view);
    _channel.read(&coordinates.x);
    _channel.read(&coordinates.y);
    _channel.read(&heuristic);
    _channel.read(&nbFeatures);

    if (!_channel.good())
        return SOURCE_PLUGIN_CRASHED;

    if (nbFeatures == 0)
        return INVALID_ARGUMENTS;

    indexes = new unsigned int[nbFeatures];
    values = new scalar_t[nbFeatures];

    _channel.read((char*) indexes, nbFeatures * sizeof(unsigned int));

    if (!_channel.good())
    {
        delete[] indexes;
        delete[] values;
        return SOURCE_PLUGIN_CRASHED;
    }

    // Compute the features
    bool success = pPerception->computeSomeFeatures(view, coordinates, heuristic, nbFeatures, indexes, values);
    if (!success)
    {
        delete[] indexes;
        delete[] values;

        if (dynamic_cast<Perception*>(pPerception))
        {
            tError error = dynamic_cast<Perception*>(pPerception)->featuresComputer()->getLastError();
            if (error == ERROR_HEURISTIC_TIMEOUT)
                return DEST_PLUGIN_TIMEOUT;
            else if (error == ERROR_NONE)
                return INVALID_ARGUMENTS;
        }
        
        return DEST_PLUGIN_CRASHED;
    }

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add((char*) values, nbFeatures * sizeof(scalar_t));
    _channel.sendPacket();

    delete[] indexes;
    delete[] values;

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handlePerceptionViewSizeCommand()
{
    // Assertions
    assert(_pTask || _pPerception);

    IPerception* pPerception = (_pPerception ? _pPerception : _pTask->perception());

    // Retrieve the index of the view
    unsigned int view;
    if (!_channel.read(&view))
        return SOURCE_PLUGIN_CRASHED;

    // Retrieve the size of the image
    dim_t size = pPerception->viewSize(view);

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(size.width);
    _channel.add(size.height);
    _channel.sendPacket();

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxTaskProxy::handlePerceptionRoiExtentCommand()
{
    // Assertions
    assert(_pTask || _pPerception);

    IPerception* pPerception = (_pPerception ? _pPerception : _pTask->perception());
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(pPerception->roiExtent());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}
