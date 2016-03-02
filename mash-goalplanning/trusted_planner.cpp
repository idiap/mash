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


/** @file   trusted_planner.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'TrustedPlanner' class
*/

#include "trusted_planner.h"
#include <mash-goalplanning/task.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <sstream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

TrustedPlanner::TrustedPlanner()
: _pManager(0), _pPlanner(0), _lastError(ERROR_NONE)
{
    _outStream.setVerbosityLevel(3);
}


TrustedPlanner::~TrustedPlanner()
{
    delete _pPlanner;
    delete _pManager;

    _outStream.deleteFile();
}


/********************************* MANAGEMENT *********************************/

void TrustedPlanner::configure(const std::string& strLogFolder,
                               const std::string& strReportFolder)
{
    _strLogFolder = strLogFolder;
    _strReportFolder = strReportFolder;

    if (!StringUtils::endsWith(_strLogFolder, "/"))
        _strLogFolder += "/";

    if (!StringUtils::endsWith(_strReportFolder, "/"))
        _strReportFolder += "/";
    
    _outStream.open("TrustedPredictor",
                    _strLogFolder + "TrustedPredictor_$TIMESTAMP.log",
                    200 * 1024);
}


/**************************** CLASSIFIER MANAGEMENT ***************************/

bool TrustedPlanner::setPlannersFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(!_pManager);
    assert(!_pPlanner);
    
    _outStream << "Goal-planners folder: " << strPath << endl;
    
    // Create the classifiers manager
    _pManager = new PlannersManager(strPath);
    
    return true;
}


bool TrustedPlanner::loadPlannerPlugin(const std::string& strName,
                                       const std::string& strModelFile)
{
    // Assertions
    assert(!strName.empty());
    assert(_pManager);
    assert(!_pPlanner);

    _outStream << "Loading goal-planner '" << strName << "'" << endl;
    
    // Load the dynamic library
    DYNLIB_HANDLE handle = _pManager->loadDynamicLibrary(strName);
    if (!handle)
    {
        string desc = _pManager->getLastErrorDescription();
        if (desc.empty())
            desc = getErrorDescription(_pManager->getLastError());
        
        _outStream << desc << endl;
        return false;
    }

    // Instantiate and initialize the classifier
    _pPlanner = _pManager->create(strName);

    if (!_pPlanner)
    {
        _outStream << getErrorDescription(_pManager->getLastError()) << endl;
        return false;
    }

    _pPlanner->outStream.setVerbosityLevel(1);
    _pPlanner->outStream.open("Predictor", _strLogFolder + "Predictor_$TIMESTAMP.log");

    _pPlanner->writer.open(_strReportFolder + "predictor.data");
    
    // Load the input model
    if (!strModelFile.empty())
        _inModel.open(strModelFile);

    // Setup the output model
    _outModel.create(_strReportFolder + "predictor.model");
    
    return true;
}


void TrustedPlanner::setNotifier(INotifier* pNotifier)
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    
    _pPlanner->notifier.useNotifier(pNotifier);
}


/*********************************** METHODS **********************************/

bool TrustedPlanner::setSeed(unsigned int seed)
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);
    
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> SET_SEED " << seed << endl;
    
    // Set the seed of the goal-planner
    _pPlanner->generator.setSeed(seed);

    return true;
}


bool TrustedPlanner::setup(const tExperimentParametersList& parameters)
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> SETUP " << parameters.size() << " ..." << endl;
    
    // Tell the goal-planner to setup itself using the parameters
    if (!_pPlanner->setup(parameters))
    {
        _lastError = ERROR_PLANNER_SETUP_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


bool TrustedPlanner::loadModel(IPerception* perception)
{
    // Assertions
    assert(perception);
    assert(_pManager);
    assert(_pPlanner);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> LOAD_MODEL " << _inModel.getFileName() << endl;

    if (!_inModel.isReadable())
    {
        _lastError = ERROR_PLANNER_MODEL_LOADING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    // Set the seed of the classifier
    _pPlanner->generator.setSeed(_inModel.predictorSeed());

    // Inform the model about the available heuristics
    for (unsigned int i = 0; i < perception->nbHeuristics(); ++i)
        _inModel.addHeuristic(i, perception->heuristicName(i));

    if (!_inModel.lockHeuristics())
    {
        _lastError = ERROR_PLANNER_MODEL_MISSING_HEURISTIC;
        _outStream << getErrorDescription(_lastError) << endl;

        tStringList missing = _inModel.missingHeuristics();
        for (unsigned int i = 0; i < missing.size(); ++i)
            _outStream << "    - " << missing[i] << endl;

        return false;
    }

    // Inform the Input Set about the heuristics that are already in the model
    Perception* pPerception = dynamic_cast<Perception*>(perception);
    if (pPerception)
    {
        for (unsigned int i = 0; i < _inModel.nbHeuristics(); ++i)
            pPerception->markHeuristicAsUsedByModel(_inModel.fromModel(i));
    }

    // Tell the classifier to load the model
    if (!_pPlanner->loadModel(_inModel))
    {
        _lastError = ERROR_CLASSIFIER_MODEL_LOADING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


bool TrustedPlanner::learn(ITask* task)
{
    // Assertions
    assert(task);
    assert(_pManager);
    assert(_pPlanner);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> LEARN" << endl;

    // Tell the goal-planner to train itself
    if (!_pPlanner->learn(task))
    {
        _lastError = ERROR_PLANNER_LEARNING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


bool TrustedPlanner::chooseAction(IPerception* perception, unsigned int* action)
{
    // Assertions
    assert(perception);
    assert(action);
    assert(_pManager);
    assert(_pPlanner);
    
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> CHOOSE_ACTION" << endl;

    // Tell the goal-planner to choose an action
    *action = _pPlanner->chooseAction(perception);
    
    return true;
}


bool TrustedPlanner::reportFeaturesUsed(IPerception* perception, tFeatureList &list)
{
    // Assertions
    assert(perception);
    assert(_pManager);
    assert(_pPlanner);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> REPORT_FEATURES_USED" << endl;

    // Ask the goal-planner about the features used
    if (!_pPlanner->reportFeaturesUsed(list))
    {
        _lastError = ERROR_PLANNER_REPORTING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    // Inform the model about the used heuristics
    if (_outModel.isWritable())
    {
        _outModel.setPredictorSeed(_pPlanner->generator.seed());
        
        tFeatureList::iterator iter, iterEnd;
        for (iter = list.begin(), iterEnd = list.end(); iter != iterEnd; ++iter)
        {
            _outModel.addHeuristic(iter->heuristic, perception->heuristicName(iter->heuristic),
                                   perception->heuristicSeed(iter->heuristic));
        }
        
        _outModel.lockHeuristics();
    }
        
    return true;
}


bool TrustedPlanner::saveModel()
{
    // Assertions
    assert(_pManager);
    assert(_pPlanner);

    if (getLastError() != ERROR_NONE)
    {
        _lastError = ERROR_PLANNER_MODEL_SAVING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    _outStream << "> SAVE_MODEL" << endl;

    if (!_outModel.isWritable())
        return false;

    // Tell the goal-planner to save the model
    if (!_pPlanner->saveModel(_outModel))
    {
        _lastError = ERROR_PLANNER_MODEL_SAVING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


tError TrustedPlanner::getLastError()
{
    return (((_lastError != ERROR_NONE) || !_pManager) ? _lastError : _pManager->getLastError());
}
