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


/** @file   goalplanning_task.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'GoalPlanningTask' class
*/

#include "goalplanning_task.h"
#include "listener.h"
#include <mash-goalplanning/perception.h>
#include <mash-utils/errors.h>
#include <mash/sandboxed_heuristics_set.h>
#include <mash/trusted_heuristics_set.h>
#include <mash-goalplanning/sandboxed_planner.h>
#include <mash-goalplanning/trusted_planner.h>
#include <mash-instrumentation/sandboxed_instruments_set.h>
#include <mash-instrumentation/trusted_instruments_set.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

GoalPlanningTask::GoalPlanningTask(Listener* pListener)
: TaskController(pListener), _pPlannerDelegate(0), _bUseModel(false)
{
    setGlobalSeed(time(0));
}


GoalPlanningTask::~GoalPlanningTask()
{
    delete _pPlannerDelegate;
}


/********************** IMPLEMENTATION OF TaskController **********************/

tError GoalPlanningTask::setup(const tTaskControllerConfiguration& configuration)
{
    // Assertions
    assert(!_pPlannerDelegate);
    assert(!_pInstrumentsSet);

    // Creates the Heuristics Set
    if (configuration.heuristicsSandboxConfiguration)
    {
        SandboxedHeuristicsSet* pHeuristicsSet = new SandboxedHeuristicsSet();
        getFeaturesComputer()->setHeuristicsSet(pHeuristicsSet);

        if (!pHeuristicsSet->createSandbox(*configuration.heuristicsSandboxConfiguration))
            return pHeuristicsSet->getLastError();
    }
    else
    {
        TrustedHeuristicsSet* pHeuristicsSet = new TrustedHeuristicsSet();
        getFeaturesComputer()->setHeuristicsSet(pHeuristicsSet);

        pHeuristicsSet->configure(configuration.strLogFolder);
    }

    if (!getFeaturesComputer()->heuristicsSet()->setHeuristicsFolder(configuration.strHeuristicsFolder))
        return getFeaturesComputer()->getLastError();

    // Creates the Planner Delegate
    if (configuration.predictorSandboxConfiguration)
    {
        SandboxedPlanner* pDelegate = new SandboxedPlanner();
        _pPlannerDelegate = pDelegate;

        if (!pDelegate->createSandbox(*configuration.predictorSandboxConfiguration))
            return pDelegate->getLastError();
    }
    else
    {
        TrustedPlanner* pDelegate = new TrustedPlanner();
        _pPlannerDelegate = pDelegate;

        pDelegate->configure(configuration.strLogFolder, configuration.strReportFolder);
    }

    if (!_pPlannerDelegate->setPlannersFolder(configuration.strPredictorsFolder))
        return _pPlannerDelegate->getLastError();

    // Creates the Instruments Set
    if (configuration.instrumentsSandboxConfiguration)
    {
        SandboxedInstrumentsSet* pInstrumentsSet = new SandboxedInstrumentsSet();
        _pInstrumentsSet = pInstrumentsSet;

        if (!pInstrumentsSet->createSandbox(*configuration.instrumentsSandboxConfiguration))
            return pInstrumentsSet->getLastError();
    }
    else
    {
        TrustedInstrumentsSet* pInstrumentsSet = new TrustedInstrumentsSet();
        _pInstrumentsSet = pInstrumentsSet;

        pInstrumentsSet->configure(configuration.strLogFolder, configuration.strReportFolder);
    }

    if (!_pInstrumentsSet->setInstrumentsFolder(configuration.strInstrumentsFolder))
        return _pInstrumentsSet->getLastError();

    // Inform the Percpetion about its listener
    _listener.pInstrumentsSet = _pInstrumentsSet;
    dynamic_cast<Perception*>(_task.perception())->setListener(&_listener);

    if (!configuration.strCaptureFolder.empty())
        _task.setCaptureFolder(configuration.strCaptureFolder);

    return ERROR_NONE;
}


void GoalPlanningTask::setGlobalSeed(unsigned int seed)
{
    RandomNumberGenerator globalGenerator;
    
    globalGenerator.setSeed(seed);
    
    for (unsigned int i = 0; i < COUNT_SEEDS; ++i)
        _seeds[i] = globalGenerator.randomize();

    getFeaturesComputer()->setSeed(_seeds[SEED_HEURISTICS]);
}


Mash::tError GoalPlanningTask::setClient(Mash::Client* pClient)
{
    assert(pClient);
    
    return _task.getController()->setClient(pClient);
}


Mash::Client* GoalPlanningTask::getClient()
{
    return static_cast<Perception*>(_task.perception())->getController()->getClient();
}


GoalPlanningTask::tResult GoalPlanningTask::setParameters(const Mash::tExperimentParametersList& parameters)
{
    // Send its global seed to the application server
    string strResponse;
    ArgumentsList args;
    
    if (!getClient()->sendCommand("USE_GLOBAL_SEED", ArgumentsList((int) _seeds[SEED_APPSERVER])))
        return tResult(ERROR_NETWORK_REQUEST_FAILURE);

    if (!getClient()->waitResponse(&strResponse, &args))
        return tResult(ERROR_NETWORK_RESPONSE_FAILURE);
    
    if (strResponse != "OK")
        return tResult(ERROR_APPSERVER_UNEXPECTED_RESPONSE, strResponse + " (expected: OK)");
    
    
    tExperimentParametersList filtered = parameters;
    
    // Retrieves the name of the goal
    tExperimentParametersList::iterator iter = filtered.find("GOAL_NAME");
    if ((iter == filtered.end()) || (iter->second.size() != 1))
        return tResult(ERROR_EXPERIMENT_PARAMETER, "No GOAL_NAME parameter found");
    
    string strGoal = iter->second.getString(0);
    filtered.erase(iter);


    // Retrieves the name of the environment
    iter = filtered.find("ENVIRONMENT_NAME");
    if ((iter == filtered.end()) || (iter->second.size() != 1))
        return tResult(ERROR_EXPERIMENT_PARAMETER, "No ENVIRONMENT_NAME parameter found");

    string strEnvironment = iter->second.getString(0);
    filtered.erase(iter);
    
    tError error = _task.getController()->selectTask(strGoal, strEnvironment, filtered);
    if (error != ERROR_NONE)
        return tResult(error, _task.getController()->getLastError());


    // Retrieves the size of the views
    int viewSize = -1;

    iter = filtered.find("VIEW_SIZE");
    if ((iter != filtered.end()) && (iter->second.size() == 1))
    {
        viewSize = iter->second.getInt(0);

        if (viewSize <= 1)
            return tResult(ERROR_EXPERIMENT_PARAMETER, "Invalid size of the views");
    }


    // Retrieves the size of the region-of-interest
    int roiExtent = -1;
    
    iter = filtered.find("ROI_SIZE");
    if ((iter != filtered.end()) && (iter->second.size() == 1))
    {
        int roiSize = iter->second.getInt(0);

        if (roiSize <= 1)
            return tResult(ERROR_EXPERIMENT_PARAMETER, "Invalid size of the region-of-interest");

        if ((viewSize > 0) && (roiSize > viewSize))
            roiSize = viewSize;

        if ((roiSize % 2) == 0)
            --roiSize;

        roiExtent = (roiSize - 1) >> 1;
    }


    static_cast<Perception*>(_task.perception())->setup(viewSize, roiExtent);
    
    return tResult();
}


GoalPlanningTask::tResult GoalPlanningTask::loadPredictor(const std::string strName,
                                                          const std::string& strModelFile,
                                                          const std::string& strInternalDataFile,
                                                          unsigned int seed)
{
    // Assertions
    assert(_pPlannerDelegate);
    
    if (!_pPlannerDelegate->loadPlannerPlugin(strName, strModelFile))
    {
        if (_pPlannerDelegate->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED)
            return tResult(ERROR_PLANNER_CRASHED);
        else
            return tResult(_pPlannerDelegate->getLastError());
    }

    _pPlannerDelegate->setNotifier(&_notifier);

    _pPlannerDelegate->setSeed((seed == 0) ? _seeds[SEED_PLANNER] : seed);

    _bUseModel = !strModelFile.empty();
    _bPredictorLoaded = true;

    return tResult();
}


bool GoalPlanningTask::setupPredictor(const Mash::tExperimentParametersList& parameters)
{
    // Assertions
    assert(_pPlannerDelegate);
    
    return _pPlannerDelegate->setup(parameters);
}


Mash::FeaturesComputer* GoalPlanningTask::getFeaturesComputer()
{
    return static_cast<Perception*>(_task.perception())->featuresComputer();
}


ServerListener::tAction GoalPlanningTask::train()
{
    // Assertions
    assert(_pListener);
    assert(_pInstrumentsSet);
    assert(_pPlannerDelegate);

    // Create the instruments if necessary
    tError error = createInstruments();
    if (error != ERROR_NONE)
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(error)))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Initialize the set of heuristics
    FeaturesComputer* pFeaturesComputer = getFeaturesComputer();
    if (!pFeaturesComputer->initialized())
    {
        if (!pFeaturesComputer->init(_task.perception()->nbViews(), static_cast<Perception*>(_task.perception())->roiExtent()))
        {
            if (!_pListener->sendResponse("ERROR", getErrorDescription(pFeaturesComputer->getLastError())))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }
    }

    // If necessary, load the predictor model
    if (_bUseModel)
    {
        if (!_pPlannerDelegate->loadModel(_task.perception()))
        {
            if (!_pListener->sendResponse("ERROR", ArgumentsList("Failed to load the predictor model")))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }
    }

    // Notify the instruments about the start of the experiment
    if (!_pInstrumentsSet->onExperimentStarted(&_task))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Notify the instruments about the start of the learning
    if (!_pInstrumentsSet->onPlannerLearningStarted(&_task))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Train the goal-planner
    if (!_pPlannerDelegate->learn(&_task))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pPlannerDelegate->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Notify the instruments
    Mash::tResult result = _task.result();
    if (!_pInstrumentsSet->onPlannerLearningDone(&_task, result))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Process the features used by the predictor
    bool bSuccess = false;
    ServerListener::tAction action = processFeaturesUsed(bSuccess);
    if (!bSuccess)
        return action;

    // Sends a response to the client
    string strResult = "NONE";
    if (result == RESULT_GOAL_REACHED)
        strResult = "GOAL_REACHED";
    else if (result == RESULT_TASK_FAILED)
        strResult = "TASK_FAILED";

    if (!_pListener->sendResponse("TRAIN_RESULT", strResult))
        return ServerListener::ACTION_CLOSE_CONNECTION;

    return ServerListener::ACTION_NONE;
}


Mash::ServerListener::tAction GoalPlanningTask::test()
{
    // Assertions
    assert(_pListener);
    assert(_pInstrumentsSet);
    assert(_pPlannerDelegate);
    
    // Reset the task
    _task.reset();

    // Notify the instruments
    if (!_pInstrumentsSet->onPlannerTestStarted(&_task))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Test
    scalar_t score = 0.0f;
    Mash::tResult result = RESULT_NONE;
    for (unsigned int i = 0; i < 1000; ++i)
    {
        unsigned int action;
        
        if (!_pPlannerDelegate->chooseAction(_task.perception(), &action))
        {
            if (!_pListener->sendResponse("ERROR", getErrorDescription(_pPlannerDelegate->getLastError())))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }
        
        scalar_t reward = 0.0f;
        _task.performAction(action, &reward);
        
        score += reward;
        
        result = _task.result();

        // Notify the instruments
        if (!_pInstrumentsSet->onPlannerActionChoosen(&_task, action, reward, result))
        {
            if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }

        if (result != RESULT_NONE)
            break;
    }

    // Notify the instruments
    if (!_pInstrumentsSet->onPlannerTestDone(&_task, score, result))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Sends a response to the client
    string strResult = "NONE";
    if (result == RESULT_GOAL_REACHED)
        strResult = "GOAL_REACHED";
    else if (result == RESULT_TASK_FAILED)
        strResult = "TASK_FAILED";
    
    if (!_pListener->sendResponse("TEST_RESULT", strResult))
        return ServerListener::ACTION_CLOSE_CONNECTION;

    if (!_pListener->sendResponse("TEST_SCORE", score))
        return ServerListener::ACTION_CLOSE_CONNECTION;

    return ServerListener::ACTION_NONE;
}


bool GoalPlanningTask::getFeaturesUsed(Mash::tFeatureList &list)
{
    // Assertions
    assert(_pPlannerDelegate);

    // Ask the goal-planner
    return _pPlannerDelegate->reportFeaturesUsed(_task.perception(), list);
}


bool GoalPlanningTask::savePredictorModel()
{
    // Assertions
    assert(_pPlannerDelegate);

    // Ask the goal-planner
    return _pPlannerDelegate->saveModel();
}


Mash::ServerListener::tAction GoalPlanningTask::reportErrors()
{
    // Assertions
    assert(_pListener);

    // Retrieve the errors
    SandboxedPlanner* pSandboxedPlanner = dynamic_cast<SandboxedPlanner*>(_pPlannerDelegate);

    tError planner_error    = (_pPlannerDelegate ? _pPlannerDelegate->getLastError() : ERROR_NONE);
    tError heuristic_error  = (getFeaturesComputer() ? getFeaturesComputer()->getLastError() : ERROR_NONE);
    tError instrument_error = (_pInstrumentsSet ? _pInstrumentsSet->getLastError() : ERROR_NONE);
    
    if (planner_error == ERROR_CHANNEL_SLAVE_CRASHED)
        planner_error = ERROR_PLANNER_CRASHED;

    if (heuristic_error == Mash::ERROR_CHANNEL_SLAVE_CRASHED)
        heuristic_error = Mash::ERROR_HEURISTIC_CRASHED;

    if (instrument_error == ERROR_CHANNEL_SLAVE_CRASHED)
        instrument_error = ERROR_INSTRUMENT_CRASHED;


    // Test if the goal-planner crashed
    if (planner_error == ERROR_PLANNER_CRASHED)
    {
        ServerListener::tAction action = sendErrorReport(
                             "PREDICTOR_CRASH", ArgumentsList(),
                             (pSandboxedPlanner ? pSandboxedPlanner->getContext() : ""),
                             true, (pSandboxedPlanner ? pSandboxedPlanner->getStackTrace() : ""));
        
        if (action != ServerListener::ACTION_NONE)
            return action;
    }

    // Test if the goal-planner used a forbidden system call
    else if (planner_error == ERROR_SANDBOX_FORBIDDEN_SYSTEM_CALL)
    {
        assert(pSandboxedPlanner);
        
        ServerListener::tAction action = sendErrorReport(
                             "PREDICTOR_ERROR",
                             getErrorDescription(planner_error) + ": " + pSandboxedPlanner->sandboxController()->getLastErrorDetails(),
                             pSandboxedPlanner->getContext());
        
        if (action != ServerListener::ACTION_NONE)
            return action;
    }

    // Test if the warden failed
    else if (planner_error == ERROR_WARDEN)
    {
        assert(pSandboxedPlanner);

        ServerListener::tAction action = sendErrorReport(
                             "PREDICTOR_ERROR",
                             pSandboxedPlanner->sandboxController()->getLastErrorDetails(),
                             pSandboxedPlanner->getContext());
        
        if (action != ServerListener::ACTION_NONE)
            return action;
    }

    // Other goal-planner errors
    else if ((planner_error != ERROR_NONE) && (planner_error != ERROR_HEURISTIC_TIMEOUT) &&
             (planner_error != ERROR_HEURISTIC_CRASHED))
    {
        ServerListener::tAction action = sendErrorReport(
                             "PREDICTOR_ERROR",
                             getErrorDescription(planner_error),
                             (pSandboxedPlanner ? pSandboxedPlanner->getContext() : ""));
        
        if (action != ServerListener::ACTION_NONE)
            return action;
    }


    // Process the error of the heuristics (if any)
    if (heuristic_error != ERROR_NONE)
    {
        ServerListener::tAction action = sendHeuristicsErrorReport(getFeaturesComputer(),
                                                                   heuristic_error);
        if (action != ServerListener::ACTION_NONE)
            return action;
    }


    // Process the error of the instruments (if any)
    if (instrument_error != ERROR_NONE)
    {
        ServerListener::tAction action = sendInstrumentsErrorReport(instrument_error);
        if (action != ServerListener::ACTION_NONE)
            return action;
    }

    
    // No error to report
    if ((planner_error == ERROR_NONE) && (heuristic_error == ERROR_NONE) && (instrument_error == ERROR_NONE))
    {
        if (!_pListener->sendResponse("NO_ERROR", ArgumentsList()))
            return ServerListener::ACTION_CLOSE_CONNECTION;
    }

    return ServerListener::ACTION_NONE;
}


unsigned int GoalPlanningTask::getNbLogFiles()
{
    // Assertions
    assert(_pPlannerDelegate);

    SandboxedHeuristicsSet* pSandboxedHeuristicsSet = dynamic_cast<SandboxedHeuristicsSet*>(getFeaturesComputer()->heuristicsSet());
    SandboxedPlanner*       pSandboxedPlanner       = dynamic_cast<SandboxedPlanner*>(_pPlannerDelegate);

    return TaskController::getNbLogFiles() +
           (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->sandboxController()->getNbLogFiles() : 0) +
           (pSandboxedPlanner ? pSandboxedPlanner->sandboxController()->getNbLogFiles() : 0);
}


int GoalPlanningTask::getLogFileContent(unsigned int index, std::string& strName,
                                        unsigned char** pBuffer, int64_t max_size)
{
    // Assertions
    assert(_pPlannerDelegate);

    unsigned int nb = TaskController::getNbLogFiles();
    if (index < nb)
        return TaskController::getLogFileContent(index, strName, pBuffer, max_size);

    index -= nb;


    SandboxedHeuristicsSet* pSandboxedHeuristicsSet = dynamic_cast<SandboxedHeuristicsSet*>(getFeaturesComputer()->heuristicsSet());
    nb = (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->sandboxController()->getNbLogFiles() : 0);
    if (index < nb)
        return pSandboxedHeuristicsSet->sandboxController()->getLogFileContent(index, strName, pBuffer, max_size);

    index -= nb;


    SandboxedPlanner* pSandboxedPlanner = dynamic_cast<SandboxedPlanner*>(_pPlannerDelegate);
    nb = (pSandboxedPlanner ? pSandboxedPlanner->sandboxController()->getNbLogFiles() : 0);
    if (index < nb)
        return pSandboxedPlanner->sandboxController()->getLogFileContent(index, strName, pBuffer, max_size);

    
    *pBuffer = 0;
    strName = "";
    return 0;
}


void GoalPlanningTask::fillReport(const std::string& strReportFolder)
{
}
