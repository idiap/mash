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


/** @file   trusted_instruments_set.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'TrustedInstrumentsSet' class
*/

#include "trusted_instruments_set.h"
#include <mash-classification/classifier_input_set.h>
#include <mash-goalplanning/task.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <sstream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

TrustedInstrumentsSet::TrustedInstrumentsSet()
: _pManager(0), _lastError(ERROR_NONE)
{
    _outStream.setVerbosityLevel(3);
}


TrustedInstrumentsSet::~TrustedInstrumentsSet()
{
    // Destroy the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        delete iter->pInstrument;

    // Destroy the instruments manager
    delete _pManager;

    _outStream.deleteFile();
}


/********************************* MANAGEMENT *********************************/

void TrustedInstrumentsSet::configure(const std::string& strLogFolder,
                                      const std::string& strReportFolder)
{
    _strReportFolder = strReportFolder;

    if (!StringUtils::endsWith(_strReportFolder, "/"))
        _strReportFolder += "/";


    string path = strLogFolder;
    if (!StringUtils::endsWith(path, "/"))
        path += "/";

    _outStream.open("TrustedInstrumentsSet",
                    path + "TrustedInstrumentsSet_$TIMESTAMP.log",
                    200 * 1024);
}


/**************************** INSTRUMENTS MANAGEMENT **************************/

bool TrustedInstrumentsSet::setInstrumentsFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(!_pManager);
    assert(_instruments.empty());
    
    _outStream << "Instruments folder: " << strPath << endl;
    
    // Create the Instruments manager
    _pManager = new InstrumentsManager(strPath);
    
    return true;
}


int TrustedInstrumentsSet::loadInstrumentPlugin(const std::string& strName)
{
    // Assertions
    assert(!strName.empty());
    assert(_pManager);
 
    _outStream << "Loading instrument plugin '" << strName << "'" << endl;
    
    DYNLIB_HANDLE handle = _pManager->loadDynamicLibrary(strName);
    if (!handle)
    {
        string desc = _pManager->getLastErrorDescription();
        if (desc.empty())
            desc = getErrorDescription(_pManager->getLastError());
        
        _outStream << desc << endl;
        return -1;
    }
    
    tInstrumentInfos infos;
    infos.strName = strName;
    infos.pInstrument = 0;
    
    _instruments.push_back(infos);
    
    return (_instruments.size() - 1);
}


bool TrustedInstrumentsSet::setupInstrument(int instrument,
                                            const tExperimentParametersList& parameters)
{
    // Assertions
    assert(instrument >= 0);
    assert(_pManager);

    if (instrument >= _instruments.size())
        return false;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> SETUP " << " " << instrument << " " << parameters.size() << " ..." << endl;

    _instruments[instrument].parameters = parameters;

    return true;
}


bool TrustedInstrumentsSet::createInstruments()
{
    // Assertions
    assert(_pManager);

    tInstrumentsIterator iter, iterEnd;

    _outStream << "Creation of the objects defined in the plugins..." << endl;

    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
    {
        _outStream << "--- Creation of '" << iter->strName << "'" << endl;

        iter->pInstrument = _pManager->create(iter->strName);
        if (!iter->pInstrument)
        {
            _outStream << getErrorDescription(_pManager->getLastError()) << endl;
            return false;
        }

        iter->pInstrument->writer.open(_strReportFolder + iter->strName + ".data");

        iter->pInstrument->setup(iter->parameters);
    }

    return true;
}


unsigned int TrustedInstrumentsSet::nbInstruments() const
{
    return _instruments.size();
}


int TrustedInstrumentsSet::instrumentIndex(const std::string& strName) const
{
    // Assertions
    assert(_pManager);

    tInstrumentsConstIterator iter, iterEnd;
    int counter;

    for (iter = _instruments.begin(), iterEnd = _instruments.end(), counter = 0;
         iter != iterEnd; ++iter, ++counter)
    {
        if (iter->strName == strName)
            return counter;
    }

    return -1;
}


std::string TrustedInstrumentsSet::instrumentName(int index) const
{
    // Assertions
    assert(_pManager);
    assert(index >= 0);

    if (index >= _instruments.size())
        return "";

    return _instruments[index].strName;
}


/******************************** GENERAL EVENTS ******************************/

bool TrustedInstrumentsSet::onExperimentDone()
{
    // Assertions
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_EXPERIMENT_DONE" << endl;

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onExperimentDone();

    return true;
}


/************************** CLASSIFIER-RELATED EVENTS *************************/

bool TrustedInstrumentsSet::onExperimentStarted(IClassifierInputSet* input_set)
{
    // Assertions
    assert(input_set);
    assert(_pManager);

    if (_instruments.empty())
        return true;
 
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_CLASSIFICATION_EXPERIMENT_STARTED " << input_set->id()
               << " " << input_set->isDoingDetection() << endl;

    ClassifierInputSet* pSet = dynamic_cast<ClassifierInputSet*>(input_set);

    if (pSet)
        pSet->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onExperimentStarted(input_set);

    if (pSet)
        pSet->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onClassifierTrainingStarted(IClassifierInputSet* input_set)
{
    // Assertions
    assert(input_set);
    assert(_pManager);

    if (_instruments.empty())
        return true;
 
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_CLASSIFIER_TRAINING_STARTED " << input_set->id()
               << " " << input_set->isDoingDetection() << endl;

    ClassifierInputSet* pSet = dynamic_cast<ClassifierInputSet*>(input_set);

    if (pSet)
        pSet->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onClassifierTrainingStarted(input_set);

    if (pSet)
        pSet->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onClassifierTrainingDone(IClassifierInputSet* input_set,
                                                     scalar_t train_error)
{
    // Assertions
    assert(input_set);
    assert(_pManager);

    if (_instruments.empty())
        return true;
 
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_CLASSIFIER_TRAINING_DONE " << train_error << endl;

    ClassifierInputSet* pSet = dynamic_cast<ClassifierInputSet*>(input_set);

    if (pSet)
        pSet->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onClassifierTrainingDone(input_set, train_error);

    if (pSet)
        pSet->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onClassifierTestStarted(IClassifierInputSet* input_set)
{
    // Assertions
    assert(input_set);
    assert(_pManager);

    if (_instruments.empty())
        return true;
 
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_CLASSIFIER_TEST_STARTED " << input_set->id()
               << " " << input_set->isDoingDetection() << endl;

    ClassifierInputSet* pSet = dynamic_cast<ClassifierInputSet*>(input_set);

    if (pSet)
        pSet->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onClassifierTestStarted(input_set);

    if (pSet)
        pSet->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onClassifierTestDone(IClassifierInputSet* input_set,
                                                 scalar_t test_error)
{
    // Assertions
    assert(input_set);
    assert(_pManager);

    if (_instruments.empty())
        return true;
 
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_CLASSIFIER_TEST_DONE " << test_error << endl;

    ClassifierInputSet* pSet = dynamic_cast<ClassifierInputSet*>(input_set);

    if (pSet)
        pSet->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onClassifierTestDone(input_set, test_error);

    if (pSet)
        pSet->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                           unsigned int image,
                                                           unsigned int original_image,
                                                           const coordinates_t& position,
                                                           const Classifier::tClassificationResults& results,
                                                           Instrument::tClassificationError error)
{
    // Assertions
    assert(input_set);
    assert(_pManager);

    if (_instruments.empty())
        return true;
 
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_CLASSIFIER_CLASSIFICATION_DONE " << image << " "
               << original_image << " " << position.x << " " << position.y
               << " " << error << " " << results.size() << " ..." << endl;

    ClassifierInputSet* pSet = dynamic_cast<ClassifierInputSet*>(input_set);

    if (pSet)
        pSet->setReadOnly(true);
       
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
    {
        iter->pInstrument->onClassifierClassificationDone(input_set, image, original_image,
                                                          position, results, error);
    }

    if (pSet)
        pSet->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onFeaturesComputedByClassifier(bool detection,
                                                           bool training,
                                                           unsigned int image,
                                                           unsigned int original_image,
                                                           const coordinates_t& coords,
                                                           unsigned int roiExtent,
                                                           unsigned int heuristic,
                                                           unsigned int nbFeatures,
                                                           unsigned int* indexes,
                                                           scalar_t* values)
{
    // Assertions
    assert(indexes);
    assert(values);
    assert(_pManager);

    if (_instruments.empty())
        return true;
 
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_FEATURES_COMPUTED_BY_CLASSIFIER " << detection << " "
               << training << " " << image << " " << original_image << " "
               << coords.x << " " << coords.y << " " << heuristic << " "
               << nbFeatures << " ..." << endl;


    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
    {
        iter->pInstrument->onFeaturesComputedByClassifier(detection, training,
                                                          image, original_image,
                                                          coords, roiExtent, heuristic,
                                                          nbFeatures, indexes, values);
    }

    return true;
}


/************************* GOALPLANNER-RELATED EVENTS *************************/

bool TrustedInstrumentsSet::onExperimentStarted(ITask* task)
{
    // Assertions
    assert(task);
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_GOALPLANNING_EXPERIMENT_STARTED" << endl;

    Task* pTask = dynamic_cast<Task*>(task);

    if (pTask)
        pTask->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onExperimentStarted(task);

    if (pTask)
        pTask->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onPlannerLearningStarted(ITask* task)
{
    // Assertions
    assert(task);
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_GOALPLANNER_LEARNING_STARTED" << endl;

    Task* pTask = dynamic_cast<Task*>(task);

    if (pTask)
        pTask->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onPlannerLearningStarted(task);

    if (pTask)
        pTask->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onPlannerLearningDone(ITask* task, tResult result)
{
    // Assertions
    assert(task);
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_GOALPLANNER_LEARNING_DONE " << result << endl;

    Task* pTask = dynamic_cast<Task*>(task);

    if (pTask)
        pTask->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onPlannerLearningDone(task, result);

    if (pTask)
        pTask->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onPlannerTestStarted(ITask* task)
{
    // Assertions
    assert(task);
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_GOALPLANNER_TEST_STARTED" << endl;

    Task* pTask = dynamic_cast<Task*>(task);

    if (pTask)
        pTask->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onPlannerTestStarted(task);

    if (pTask)
        pTask->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onPlannerTestDone(ITask* task, scalar_t score, tResult result)
{
    // Assertions
    assert(task);
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_GOALPLANNER_TEST_DONE " << score << " " << result << endl;

    Task* pTask = dynamic_cast<Task*>(task);

    if (pTask)
        pTask->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onPlannerTestDone(task, score, result);

    if (pTask)
        pTask->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onPlannerActionChoosen(ITask* task, unsigned int action,
                                                   scalar_t reward, tResult result)
{
    // Assertions
    assert(task);
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_GOALPLANNER_ACTION_CHOOSEN " << action << " "
               << reward << " " << result << endl;

    Task* pTask = dynamic_cast<Task*>(task);

    if (pTask)
        pTask->setReadOnly(true);

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onPlannerActionChoosen(task, action, reward, result);

    if (pTask)
        pTask->setReadOnly(false);

    return true;
}


bool TrustedInstrumentsSet::onFeaturesComputedByPlanner(unsigned int sequence,
                                                        unsigned int view,
                                                        unsigned int image,
                                                        const coordinates_t& coords,
                                                        unsigned int roiExtent,
                                                        unsigned int heuristic,
                                                        unsigned int nbFeatures,
                                                        unsigned int* indexes,
                                                        scalar_t* values)
{
    // Assertions
    assert(indexes);
    assert(values);
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_FEATURES_COMPUTED_BY_GOALPLANNER " << sequence << " " << view << " "
               << image << " " << coords.x << " " << coords.y << " " << heuristic
               << " " << nbFeatures << " ..." << endl;

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
    {
        iter->pInstrument->onFeaturesComputedByPlanner(sequence, view, image,
                                                       coords, roiExtent, heuristic,
                                                       nbFeatures, indexes, values);
    }

    return true;
}


/************************** PREDICTOR-RELATED EVENTS **************************/

bool TrustedInstrumentsSet::onFeatureListReported(const tFeatureList& features)
{
    // Assertions
    assert(_pManager);

    if (_instruments.empty())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> EVENT_FEATURE_LIST_REPORTED " << features.size() << " ..." << endl;

    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
        iter->pInstrument->onFeatureListReported(features);

    return true;
}


/*********************************** METHODS **********************************/

tError TrustedInstrumentsSet::getLastError()
{
    return (((_lastError != ERROR_NONE) || !_pManager) ? _lastError : _pManager->getLastError());
}
