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


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxedPlanner::SandboxedPlanner()
: _pTaskProxy(0), _pNotifierProxy(0), _lastError(ERROR_NONE)
{
}


SandboxedPlanner::~SandboxedPlanner()
{
    _outStream.deleteFile();
    
    delete _pNotifierProxy;
}


/***************************** SANDBOX MANAGEMENT *****************************/

bool SandboxedPlanner::createSandbox(const tSandboxConfiguration& configuration)
{
    _outStream.setVerbosityLevel(3);
    _outStream.open("PredictorSandboxController",
                    configuration.strLogDir + "PredictorSandboxController_$TIMESTAMP.log",
                    200 * 1024);

    _sandbox.setOutputStream(_outStream);
    _sandbox.addLogFileInfos("PredictorSandbox");

    _sandbox.addLogFileInfos("Predictor", true, true);

    if (!_sandbox.createSandbox(PLUGIN_GOALPLANNER, configuration, this))
        return false;
    
    _pNotifierProxy = new SandboxNotifierProxy(0, *_sandbox.channel());
    return true;
}


/**************************** CLASSIFIER MANAGEMENT ***************************/

bool SandboxedPlanner::setPlannersFolder(const std::string& strPath)
{
    return _sandbox.setPluginsFolder(strPath);
}


bool SandboxedPlanner::loadPlannerPlugin(const std::string& strName,
                                         const std::string& strModelFile)
{
    // Save the context (in case of crash)
    std::ostringstream str;
    str << "Method: loading" << endl;
	_strContext = str.str();

    if (_sandbox.loadPlugin(strName) != 0)
        return false;
        
    // Save the context (in case of crash)
    str.str("");
    str << "Method: constructor" << endl
        << "Parameters:" << endl;

    if (!strModelFile.empty())
    {
        str << "    - Model" << endl;
        _outStream << "< USE_MODEL " << strModelFile << " -" << endl;
    }
    else
    {
        str << "    - No model" << endl;
        _outStream << "< USE_MODEL -" << endl;
    }

	_strContext = str.str();

    // Send the infos about the model to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_USE_MODEL);
    pChannel->add(!strModelFile.empty() ? strModelFile : "-");
    pChannel->add("-");
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();

    // Create the plugins
    if (result)
        result = _sandbox.createPlugins();

    return result;
}


void SandboxedPlanner::setNotifier(INotifier* pNotifier)
{
    delete _pNotifierProxy;
    _pNotifierProxy = new SandboxNotifierProxy(pNotifier, *_sandbox.channel());
}


/*********************************** METHODS **********************************/

bool SandboxedPlanner::setSeed(unsigned int seed)
{
    if (getLastError() != ERROR_NONE)
        return false;

	_strContext = "";

    _outStream << "< SET_SEED " << " " << seed << endl;

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_PLANNER_SET_SEED);
    pChannel->add(seed);
    pChannel->sendPacket();

    if (!pChannel->good())
        return false;

    // Read the response
    return _sandbox.waitResponse();
}


bool SandboxedPlanner::setup(const tExperimentParametersList& parameters)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< SETUP " << " " << parameters.size() << " ..." << endl;

    // Save the context (in case of crash)
    std::ostringstream str;
	
    str << "Method: setup" << endl;

    if (parameters.empty())
        str << "No parameters" << endl;
    else
        str << "Parameters:" << endl;

    tExperimentParametersIterator iter, iterEnd;
    for (iter = parameters.begin(), iterEnd = parameters.end();
         iter != iterEnd; ++iter)
    {
        str << "    - " << iter->first;
        
        unsigned int nbArguments = iter->second.size();

        for (unsigned int i = 0; i < nbArguments; ++i)
            str << " " << iter->second.getString(i);
        
        str << endl;
    }

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_PLANNER_SETUP);
    pChannel->add((unsigned int) parameters.size());

    for (iter = parameters.begin(), iterEnd = parameters.end();
         iter != iterEnd; ++iter)
    {
        pChannel->add(iter->first);

        unsigned int nbArguments = iter->second.size();
        pChannel->add(nbArguments);

        for (unsigned int i = 0; i < nbArguments; ++i)
            pChannel->add(iter->second.getString(i));
    }

    pChannel->sendPacket();

    // Read the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_PLANNER_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedPlanner::loadModel(IPerception* perception)
{
    // Assertions
    assert(perception);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< LOAD_MODEL" << endl;

    // Save the context (in case of crash)
    std::ostringstream str;
    str << "Method: loadModel" << endl;
    _strContext = str.str();

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(perception, *pChannel);
    
    // Send the command to the child
    pChannel->startPacket(SANDBOX_COMMAND_LOAD_MODEL);
    pChannel->sendPacket();

    // Read the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();

    delete _pTaskProxy;
    _pTaskProxy = 0;

    return result && pChannel->good();
}


bool SandboxedPlanner::learn(ITask* task)
{
    // Assertions
    assert(task);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< LEARN" << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(task, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

    str << "Method: learn" << endl
        << "Parameters:" << endl
        << "    - Number of actions:    " << task->nbActions() << endl
        << "    - Number of views:      " << task->perception()->nbViews() << endl
        << "    - Number of heuristics: " << task->perception()->nbHeuristics() << endl
        << "    - Number of features:   " << task->perception()->nbFeaturesTotal() << endl;

    _strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_COMMAND_PLANNER_LEARN);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pTaskProxy;
    _pTaskProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_PLANNER_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedPlanner::chooseAction(IPerception* perception, unsigned int* action)
{
    // Assertions
    assert(perception);
    assert(action);
    
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< CHOOSE_ACTION " << (perception->newSequence() ? 1 : 0) << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(perception, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;
 
    str << "Method: chooseAction" << endl
        << "Parameters:" << endl
        << "    - Number of views:      " << perception->nbViews() << endl
        << "    - Number of heuristics: " << perception->nbHeuristics() << endl
        << "    - Number of features:   " << perception->nbFeaturesTotal() << endl;

    _strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_COMMAND_PLANNER_CHOOSE_ACTION);
    pChannel->add(perception->newSequence() ? 1 : 0);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    // Retrieve the action
    if (result)
        result = pChannel->read(action);
    
    delete _pTaskProxy;
    _pTaskProxy = 0;
 
    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_PLANNER_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedPlanner::reportFeaturesUsed(IPerception* perception, tFeatureList &list)
{
    // Assertions
    assert(perception);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< REPORT_FEATURES_USED" << endl;

    // Save the context (in case of crash)
    std::ostringstream str;
	
    str << "Method: reportFeaturesUsed" << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(perception, *pChannel);

    pChannel->startPacket(SANDBOX_COMMAND_PLANNER_REPORT_FEATURES_USED);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    // Retrieve the features
    if (result)
    {
        unsigned int nbFeatures = 0;
        pChannel->read(&nbFeatures);
        
        for (unsigned int i = 0; pChannel->good() && (i < nbFeatures); ++i)
        {
            unsigned int heuristic;
            unsigned int feature;
            
            if (pChannel->read(&heuristic) && pChannel->read(&feature))
                list.push_back(tFeature(heuristic, feature));
        }
    }

    delete _pTaskProxy;
    _pTaskProxy = 0;

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_PLANNER_CRASHED : ERROR_NONE;
        
    return result && pChannel->good();
}


bool SandboxedPlanner::saveModel()
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< SAVE_MODEL" << endl;

    // Save the context (in case of crash)
    std::ostringstream str;
    str << "Method: saveModel" << endl;
    _strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_SAVE_MODEL);
    pChannel->sendPacket();

    // Read the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();

    return result && pChannel->good();
}


tError SandboxedPlanner::getLastError()
{
    return (_lastError != ERROR_NONE ?
                _lastError :
                (_sandbox.getLastError() == ERROR_CHANNEL_SLAVE_CRASHED ?
                        ERROR_PLANNER_CRASHED : _sandbox.getLastError()));
}


tCommandProcessingResult SandboxedPlanner::processResponse(tSandboxMessage message)
{
    tCommandProcessingResult result = COMMAND_UNKNOWN;
    
    if (_pTaskProxy)
        result = _pTaskProxy->processResponse(message);

    if ((result == COMMAND_UNKNOWN) && _pNotifierProxy)
        result = _pNotifierProxy->processResponse(message);

    return result;
}
