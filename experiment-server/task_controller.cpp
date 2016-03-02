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


/** @file   task_controller.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'TaskController' class
*/

#include "task_controller.h"
#include "listener.h"
#include <mash-instrumentation/sandboxed_instruments_set.h>
#include <mash/sandboxed_heuristics_set.h>
#include <mash-utils/errors.h>
#include <algorithm>
#include <stdlib.h>


using namespace std;

using Mash::SandboxedInstrumentsSet;
using Mash::SandboxedHeuristicsSet;
using Mash::ServerListener;
using Mash::ArgumentsList;
using Mash::tFeatureList;
using Mash::tFeature;


/****************************** UTILITY FUNCTIONS *****************************/

bool features_less_than(const tFeature& i1, const tFeature& i2)
{
    return (i1.heuristic < i2.heuristic) || ((i1.heuristic == i2.heuristic) && (i1.feature_index < i2.feature_index));
}


bool features_equal(const tFeature& i1, const tFeature& i2)
{
    return (i1.heuristic == i2.heuristic) && (i1.feature_index == i2.feature_index);
}


/************************* CONSTRUCTION / DESTRUCTION *************************/

TaskController::TaskController(Listener* pListener)
: _pListener(pListener), _bPredictorLoaded(false), _bInstrumentsCreated(false),
  _pInstrumentsSet(0), _notifier(pListener)
{
}


TaskController::~TaskController()
{
    delete _pInstrumentsSet;
}


/**************************** METHODS TO IMPLEMENT ****************************/

unsigned int TaskController::getNbLogFiles()
{
    // Assertions
    assert(_pInstrumentsSet);

    SandboxedInstrumentsSet* pSet = dynamic_cast<SandboxedInstrumentsSet*>(_pInstrumentsSet);

    return (pSet ? pSet->sandboxController()->getNbLogFiles() : 0);
}


int TaskController::getLogFileContent(unsigned int index, std::string& strName,
                                      unsigned char** pBuffer, int64_t max_size)
{
    // Assertions
    assert(_pInstrumentsSet);

    SandboxedInstrumentsSet* pSet = dynamic_cast<SandboxedInstrumentsSet*>(_pInstrumentsSet);
    
    if (pSet)
        return pSet->sandboxController()->getLogFileContent(index, strName, pBuffer, max_size);
    
    *pBuffer = 0;
    strName = "";
    return 0;
}


/********************************** METHODS ***********************************/

TaskController::tResult TaskController::loadInstrument(const std::string strName)
{
    // Assertions
    assert(_pInstrumentsSet);

    if (_pInstrumentsSet->loadInstrumentPlugin(strName) < 0)
        return tResult(_pInstrumentsSet->getLastError());
    
    _instrumentsParameters.push_back(Mash::tExperimentParametersList());
    
    return tResult();
}


void TaskController::setInstrumentParameters(const std::string strName,
                                             const Mash::tExperimentParametersList& parameters)
{
    // Assertions
    assert(_pInstrumentsSet);

    int index = _pInstrumentsSet->instrumentIndex(strName);
    if (index >= 0)
        _instrumentsParameters[index] = parameters;
}


Mash::tError TaskController::createInstruments()
{
    // Assertions
    assert(_pInstrumentsSet);

    if (_bInstrumentsCreated)
        return Mash::ERROR_NONE;
    
    if (!_pInstrumentsSet->createInstruments())
    {
        if (_pInstrumentsSet->getLastError() == Mash::ERROR_CHANNEL_SLAVE_CRASHED)
            return Mash::ERROR_INSTRUMENT_CRASHED;
        else
            return _pInstrumentsSet->getLastError();
    }


    int i = 0;
    std::vector<Mash::tExperimentParametersList>::iterator iter, iterEnd;
    for (i = 0, iter = _instrumentsParameters.begin(), iterEnd = _instrumentsParameters.end();
         iter != iterEnd; ++iter, ++i)
    {
        if (!_pInstrumentsSet->setupInstrument(i, *iter))
        {
            if (_pInstrumentsSet->getLastError() == Mash::ERROR_CHANNEL_SLAVE_CRASHED)
                return Mash::ERROR_INSTRUMENT_CRASHED;
            else
                return _pInstrumentsSet->getLastError();
        }
    }

    _instrumentsParameters.clear();

    _bInstrumentsCreated = true;
    
    return Mash::ERROR_NONE;
}


Mash::ServerListener::tAction TaskController::sendHeuristicsErrorReport(
                                                Mash::FeaturesComputer* pFeaturesComputer,
                                                Mash::tError heuristic_error)
{
    // Assertions
    assert(_pListener);
    assert(pFeaturesComputer);

    ServerListener::tAction action = ServerListener::ACTION_NONE;
    
    SandboxedHeuristicsSet* pSandboxedHeuristicsSet =
                    dynamic_cast<SandboxedHeuristicsSet*>(pFeaturesComputer->heuristicsSet());

    // Test if a heuristic crashed
    if (heuristic_error == Mash::ERROR_HEURISTIC_CRASHED)
    {
        action = sendErrorReport("HEURISTIC_CRASH",
                                 (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->heuristicName(pSandboxedHeuristicsSet->currentHeuristic()) : ""),
                                 (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->getContext() : ""),
                                 true, (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->getStackTrace() : ""));
    }
    
    // Test if a heuristic exhausted its time budget
    else if (heuristic_error == Mash::ERROR_HEURISTIC_TIMEOUT)
    {
        action = sendErrorReport("HEURISTIC_TIMEOUT",
                                 (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->heuristicName(pSandboxedHeuristicsSet->currentHeuristic()) : ""),
                                 (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->getContext() : ""));
    }

    // Test if a heuristic used a forbidden system call
    else if (heuristic_error == Mash::ERROR_SANDBOX_FORBIDDEN_SYSTEM_CALL)
    {
        assert(pSandboxedHeuristicsSet);
        
        ArgumentsList args;
        args.add(pSandboxedHeuristicsSet->heuristicName(pSandboxedHeuristicsSet->currentHeuristic()));
        args.add(getErrorDescription(heuristic_error) + ": " + pSandboxedHeuristicsSet->sandboxController()->getLastErrorDetails());

        action = sendErrorReport("HEURISTIC_ERROR", args,
                                 (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->getContext() : ""));
    }

    // Test if the warden failed
    else if (heuristic_error == Mash::ERROR_WARDEN)
    {
        assert(pSandboxedHeuristicsSet);
        
        ArgumentsList args;
        args.add(pSandboxedHeuristicsSet->heuristicName(pSandboxedHeuristicsSet->currentHeuristic()));
        args.add(pSandboxedHeuristicsSet->sandboxController()->getLastErrorDetails());

        action = sendErrorReport("HEURISTIC_ERROR", args,
                                 (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->getContext() : ""));
    }
    
    // Other heuristic errors
    else if (heuristic_error != Mash::ERROR_NONE)
    {
        ArgumentsList args;
        
        if (pSandboxedHeuristicsSet)
        {
            args.add(pSandboxedHeuristicsSet->heuristicName(pSandboxedHeuristicsSet->currentHeuristic()));
            args.add(getErrorDescription(heuristic_error));
        }

        action = sendErrorReport("HEURISTIC_ERROR", args,
                                 (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->getContext() : ""));
    }

    return action;
}


Mash::ServerListener::tAction TaskController::sendInstrumentsErrorReport(Mash::tError instrument_error)
{
    // Assertions
    assert(_pListener);
    assert(_pInstrumentsSet);

    ServerListener::tAction action = ServerListener::ACTION_NONE;
    
    SandboxedInstrumentsSet* pSandboxedInstrumentsSet =
                    dynamic_cast<SandboxedInstrumentsSet*>(_pInstrumentsSet);


    // Test if an instrument crashed
    if (instrument_error == Mash::ERROR_INSTRUMENT_CRASHED)
    {
        action = sendErrorReport("INSTRUMENT_CRASH",
                                 (pSandboxedInstrumentsSet ? pSandboxedInstrumentsSet->instrumentName(pSandboxedInstrumentsSet->currentInstrument()) : ""),
                                 (pSandboxedInstrumentsSet ? pSandboxedInstrumentsSet->getContext() : ""),
                                 true, (pSandboxedInstrumentsSet ? pSandboxedInstrumentsSet->getStackTrace() : ""));
    }
    
    // Test if the instrument used a forbidden system call
    else if (instrument_error == Mash::ERROR_SANDBOX_FORBIDDEN_SYSTEM_CALL)
    {
        assert(pSandboxedInstrumentsSet);
        
        ArgumentsList args;
        args.add(pSandboxedInstrumentsSet->instrumentName(pSandboxedInstrumentsSet->currentInstrument()));
        args.add(getErrorDescription(instrument_error) + ": " + pSandboxedInstrumentsSet->sandboxController()->getLastErrorDetails());

        action = sendErrorReport("INSTRUMENT_ERROR", args,
                                 (pSandboxedInstrumentsSet ? pSandboxedInstrumentsSet->getContext() : ""));
    }

    // Test if the warden failed
    else if (instrument_error == Mash::ERROR_WARDEN)
    {
        assert(pSandboxedInstrumentsSet);
        
        ArgumentsList args;
        args.add(pSandboxedInstrumentsSet->instrumentName(pSandboxedInstrumentsSet->currentInstrument()));
        args.add(pSandboxedInstrumentsSet->sandboxController()->getLastErrorDetails());

        action = sendErrorReport("INSTRUMENT_ERROR", args,
                                 (pSandboxedInstrumentsSet ? pSandboxedInstrumentsSet->getContext() : ""));
    }

    // Other instrument errors
    else if (instrument_error != Mash::ERROR_NONE)
    {
        ArgumentsList args;
        
        if (pSandboxedInstrumentsSet)
        {
            args.add(pSandboxedInstrumentsSet->instrumentName(pSandboxedInstrumentsSet->currentInstrument()));
            args.add(getErrorDescription(instrument_error));
        }

        action = sendErrorReport("INSTRUMENT_ERROR", args,
                                 (pSandboxedInstrumentsSet ? pSandboxedInstrumentsSet->getContext() : ""));
    }

    return action;
}


Mash::ServerListener::tAction TaskController::sendErrorReport(
                                                const std::string& strErrorType,
                                                const Mash::ArgumentsList& args,
                                                const std::string& strContext,
                                                bool bHasStackTrace,
                                                const std::string& strStackTrace)
{
    // Assertions
    assert(_pListener);
    assert(!strErrorType.empty());
    
    if (!_pListener->sendResponse(strErrorType, args))
        return ServerListener::ACTION_CLOSE_CONNECTION;

    if (!strContext.empty() && !_pListener->sendResponse("CONTEXT", ArgumentsList(strContext)))
        return ServerListener::ACTION_CLOSE_CONNECTION;
    else if (strContext.empty() && !_pListener->sendResponse("NO_CONTEXT", ArgumentsList()))
        return ServerListener::ACTION_CLOSE_CONNECTION;

    if (bHasStackTrace)
    {
        if (!strStackTrace.empty())
        {
            if (!_pListener->sendResponse("STACKTRACE", ArgumentsList(strStackTrace)))
                return ServerListener::ACTION_CLOSE_CONNECTION;
        }
        else
        {
            if (!_pListener->sendResponse("NO_STACKTRACE", ArgumentsList()))
                return ServerListener::ACTION_CLOSE_CONNECTION;
        }
    }
    
    return ServerListener::ACTION_NONE;
}


Mash::ServerListener::tAction TaskController::processFeaturesUsed(bool &bSuccess)
{
    bSuccess = false;

    // Retrieve the list of features used by the predictor
    tFeatureList features;
    if (!getFeaturesUsed(features))
    {
        if (!_pListener->sendResponse("ERROR", ArgumentsList("Failed to retrieve the list of features used by the predictor")))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Clean it (we can't really trust the predictor here)
    sort(features.begin(), features.end(), features_less_than);
    tFeatureList::iterator iter = unique(features.begin(), features.end(), features_equal);
    features.erase(iter, features.end());

    // Notify the instruments
    _pInstrumentsSet->onFeatureListReported(features);

    bSuccess = true;

    return ServerListener::ACTION_NONE;
}
