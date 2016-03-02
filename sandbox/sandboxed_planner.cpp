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


/** @file   sandboxed_planner.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxedPlanner' class
*/

#include "sandboxed_planner.h"
#include <mash-utils/errors.h>
#include <stdlib.h>
#include <assert.h>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include "dynlibs_manager_delegate.h"
#endif


using namespace std;
using namespace Mash;


/****************************** STATIC ATTRIBUTES *****************************/

SandboxedPlanner::tCommandHandlersList SandboxedPlanner::handlers;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxedPlanner::SandboxedPlanner(const CommunicationChannel& channel,
                                   OutStream* pOutStream)
: ISandboxedObject(channel, pOutStream), _pManager(0), _pPlanner(0),
  _task(channel, pOutStream, &_wardenContext),
  _notifier(channel, pOutStream, &_wardenContext)
{
    if (handlers.empty())
    {
        handlers[SANDBOX_COMMAND_PLANNER_SET_SEED]              = &SandboxedPlanner::handleSetSeedCommand;
        handlers[SANDBOX_COMMAND_PLANNER_SETUP]                 = &SandboxedPlanner::handleSetupCommand;
        handlers[SANDBOX_COMMAND_LOAD_MODEL]                    = &SandboxedPlanner::handleLoadModelCommand;
        handlers[SANDBOX_COMMAND_PLANNER_LEARN]                 = &SandboxedPlanner::handleLearnCommand;
        handlers[SANDBOX_COMMAND_PLANNER_CHOOSE_ACTION]         = &SandboxedPlanner::handleChooseActionCommand;
        handlers[SANDBOX_COMMAND_PLANNER_REPORT_FEATURES_USED]  = &SandboxedPlanner::handleReportFeaturesUsedCommand;
        handlers[SANDBOX_COMMAND_SAVE_MODEL]                    = &SandboxedPlanner::handleSaveModelCommand;
    }

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _wardenContext.sandboxed_object            = 0;
    _wardenContext.memory_allocated            = 0;
    _wardenContext.memory_allocated_maximum    = 0;
    _wardenContext.memory_limit                = 2LL * 1024 * 1024 * 1024;
#endif
}


SandboxedPlanner::~SandboxedPlanner()
{
    // Destroy the goal-planner
    OutStream s(_pPlanner->outStream);
    DataWriter w(_pPlanner->writer);
    
    setWardenContext(&_wardenContext);
    delete _pPlanner;
    setWardenContext(0);

    // Destroy the goal-planners manager
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    wardenEnableUnsafeFree();
#endif

    delete _pManager;
}


/******************** IMPLEMENTATION OF ISandboxedObject **********************/

tError SandboxedPlanner::setPluginsFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(!_pManager);
    assert(!_pPlanner);
    
    _outStream << "Plugins folder: " << strPath << endl;
    
    // Create the goal-planners manager
    _pManager = new PlannersManager(strPath);
    
    return ERROR_NONE;
}


tError SandboxedPlanner::loadPlugin(const std::string& strName)
{
    // Assertions
    assert(!strName.empty());
    assert(_pManager);
    assert(!_pPlanner);

    _outStream << "Loading plugin '" << strName << "'" << endl;
    
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _wardenContext.exceptions = WARDEN_EXCEPTION_DLOPEN;
    DynlibsManagerDelegate delegate(&_wardenContext);
    _pManager->setDelegate(&delegate);
#endif

    DYNLIB_HANDLE handle = _pManager->loadDynamicLibrary(strName);

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _pManager->setDelegate(0);
    _wardenContext.exceptions = 0;
#endif

    if (!handle)
    {
        string desc = _pManager->getLastErrorDescription();
        if (desc.empty())
            desc = getErrorDescription(_pManager->getLastError());
        
        _outStream << desc << endl;
        return _pManager->getLastError();
    }
    
    // Save the name of the goal-planner for later (needed by createPlugins())
    _strPlannerName = strName;
    
    return ERROR_NONE;
}


tError SandboxedPlanner::createPlugins(Mash::OutStream* pOutStream,
                                       const std::vector<Mash::DataWriter>& dataWriters,
                                       const std::vector<Mash::DataWriter>& outCache,
                                       const std::vector<Mash::DataReader>& inCache,
                                       const Mash::PredictorModel& inModel,
                                       const Mash::DataReader& inInternalData,
                                       const Mash::PredictorModel& outModel,
                                       const Mash::DataWriter& outInternalData)
{
    // Assertions
    assert(_pManager);
    assert(!_pPlanner);
    assert(!_strPlannerName.empty());

    _outStream << "Creation of the objects defined in the plugins..." << endl;

    // Create the goal-planner
    _outStream << "--- Creation of '" << _strPlannerName << "'" << endl;

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    DynlibsManagerDelegate delegate(&_wardenContext);
    _pManager->setDelegate(&delegate);
#endif

    _pPlanner = _pManager->create(_strPlannerName);

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _pManager->setDelegate(0);
#endif

    if (!_pPlanner)
    {
        _outStream << getErrorDescription(_pManager->getLastError()) << endl;
        return _pManager->getLastError();
    }

    if (pOutStream)
        _pPlanner->outStream = *pOutStream;

    if (!dataWriters.empty())
        _pPlanner->writer = dataWriters[0];
    
    _pPlanner->notifier.useNotifier(&_notifier);

    _inModel  = inModel;
    _outModel = outModel;
    
    return ERROR_NONE;
}


void SandboxedPlanner::handleCommand(tSandboxMessage command)
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    assert(!handlers.empty());

    tCommandHandlersIterator iter = handlers.find(command);
    if (iter != handlers.end())
    {
        tCommandHandler handler = iter->second;
        tError result = (this->*handler)();
        
        if (result != ERROR_NONE)
        {
            _outStream << "< ERROR " << getErrorDescription(result) << endl;
            
            _channel.startPacket(SANDBOX_MESSAGE_ERROR);
            _channel.add(result);
            _channel.sendPacket();
        }
    }
    else
    {
        _channel.startPacket(SANDBOX_MESSAGE_UNKNOWN_COMMAND);
        _channel.sendPacket();
    }
}


/**************************** COMMAND HANDLERS ********************************/

tError SandboxedPlanner::handleSetSeedCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    
    // Retrieve the seed
    unsigned int seed;
    if (!_channel.read(&seed))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> SET_SEED " << seed << endl;
    
    // Set the seed of the classifier
    _pPlanner->generator.setSeed(seed);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedPlanner::handleSetupCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    
    // Retrieve the parameters
    unsigned int nbParameters = 0;
    _channel.read(&nbParameters);
    
    tExperimentParametersList parameters;
    
    for (unsigned int i = 0; _channel.good() && (i < nbParameters); ++i)
    {
        string strName;
        if (!_channel.read(&strName))
            break;
        
        ArgumentsList arguments;

        unsigned int nbArguments;
        _channel.read(&nbArguments);

        for (unsigned int j = 0; _channel.good() && (j < nbArguments); ++j)
        {
            string strValue;
            if (_channel.read(&strValue))
                arguments.add(strValue);
        }

        if (_channel.good())
            parameters[strName] = arguments;
    }
    
    if (!_channel.good())
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> SETUP " << nbParameters << " ..." << endl;
    
    // Tell the classifier to setup itself using the parameters
    setWardenContext(&_wardenContext);
    if (!_pPlanner->setup(parameters))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_PLANNER_SETUP_FAILED) << endl;
        return ERROR_PLANNER_SETUP_FAILED;
    }
    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedPlanner::handleLoadModelCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    
    _outStream << "> LOAD_MODEL" << endl;
    
    if (!_inModel.isReadable())
    {
        _outStream << getErrorDescription(ERROR_PLANNER_MODEL_LOADING_FAILED) << endl;
        return ERROR_PLANNER_MODEL_LOADING_FAILED;
    }
    
    // Inform the model about the available heuristics
    SandboxPerception* pPerception = dynamic_cast<SandboxPerception*>(_task.perception());
    for (unsigned int i = 0; i < pPerception->nbHeuristics(); ++i)
        _inModel.addHeuristic(i, pPerception->heuristicName(i));
    
    if (!_inModel.lockHeuristics())
    {
        _outStream << getErrorDescription(ERROR_PLANNER_MODEL_MISSING_HEURISTIC) << endl;

        tStringList missing = _inModel.missingHeuristics();
        for (unsigned int i = 0; i < missing.size(); ++i)
            _outStream << "    - " << missing[i] << endl;

        return ERROR_PLANNER_MODEL_MISSING_HEURISTIC;
    }

    // Inform the Perception about the heuristics that are already in the model
    for (unsigned int i = 0; i < _inModel.nbHeuristics(); ++i)
        pPerception->markHeuristicAsUsedByModel(_inModel.fromModel(i));
    
    // Tell the goal-planner to load the model
    setWardenContext(&_wardenContext);
    if (!_pPlanner->loadModel(_inModel))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_PLANNER_MODEL_LOADING_FAILED) << endl;
        return ERROR_PLANNER_MODEL_LOADING_FAILED;
    }
    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedPlanner::handleLearnCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    
    _outStream << "> LEARN" << endl;
    
    // Tell the goal-planner to learn to solve the task
    setWardenContext(&_wardenContext);
    if (!_pPlanner->learn(&_task))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_PLANNER_LEARNING_FAILED) << endl;
        return ERROR_PLANNER_LEARNING_FAILED;
    }
    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedPlanner::handleChooseActionCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    
    // Retrieve the parameters
    unsigned int newSequence = 0;
    _channel.read(&newSequence);

    _outStream << "> CHOOSE_ACTION " << newSequence << endl;

    dynamic_cast<SandboxPerception*>(_task.perception())->setNewSequence(newSequence == 1);

    // Tell the goal-planner to choose an action
    setWardenContext(&_wardenContext);
    unsigned int action = _pPlanner->chooseAction(_task.perception());
    setWardenContext(0);

    // Send the results
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(action);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedPlanner::handleReportFeaturesUsedCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);

    _outStream << "> REPORT_FEATURES_USED" << endl;

    // Ask the classifier about the features used
    setWardenContext(&_wardenContext);
    tFeatureList* features = new tFeatureList();
    if (!_pPlanner->reportFeaturesUsed(*features))
    {
        delete features;
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_PLANNER_REPORTING_FAILED) << endl;
        return ERROR_PLANNER_REPORTING_FAILED;
    }
    setWardenContext(0);

    // Inform the model about the used heuristics
    tFeatureList::iterator iter, iterEnd;
    if (_outModel.isWritable())
    {
        _outModel.setPredictorSeed(_pPlanner->generator.seed());
        
        for (iter = features->begin(), iterEnd = features->end(); iter != iterEnd; ++iter)
        {
            _outModel.addHeuristic(iter->heuristic, _task.perception()->heuristicName(iter->heuristic),
                                   _task.perception()->heuristicSeed(iter->heuristic));
        }

        _outModel.lockHeuristics();
    }

    // Send the features
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);

    unsigned int nbFeatures = features->size();
    _channel.add((unsigned int) features->size());

    for (iter = features->begin(), iterEnd = features->end();
         _channel.good() && (iter != iterEnd); ++iter)
    {
        _channel.add(iter->heuristic);
        _channel.add(iter->feature_index);
    }

    _channel.sendPacket();

    setWardenContext(&_wardenContext);
    delete features;
    setWardenContext(0);

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedPlanner::handleSaveModelCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    
    _outStream << "> SAVE_MODEL" << endl;
    
    if (!_outModel.isWritable())
    {
        _outStream << getErrorDescription(ERROR_PLANNER_MODEL_SAVING_FAILED) << endl;
        return ERROR_PLANNER_MODEL_SAVING_FAILED;
    }
    
    // Tell the classifier to save the model
    setWardenContext(&_wardenContext);
    if (!_pPlanner->saveModel(_outModel))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_PLANNER_MODEL_SAVING_FAILED) << endl;
        return ERROR_PLANNER_MODEL_SAVING_FAILED;
    }
    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}
