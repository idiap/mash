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


/** @file   sandboxed_instruments_set.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxedInstrumentsSet' class
*/

#include "sandboxed_instruments_set.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-sandboxing/declarations.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <sstream>


using namespace std;
using namespace Mash;
using namespace Mash::SandboxControllerDeclarations;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxedInstrumentsSet::SandboxedInstrumentsSet()
: _pInputSetProxy(0), _pTaskProxy(0), _currentInstrument(-1),
  _lastError(ERROR_NONE)
{
}


SandboxedInstrumentsSet::~SandboxedInstrumentsSet()
{
    _outStream.deleteFile();
}


/***************************** SANDBOX MANAGEMENT *****************************/

bool SandboxedInstrumentsSet::createSandbox(const tSandboxConfiguration& configuration)
{
    _outStream.setVerbosityLevel(3);
    _outStream.open("InstrumentsSandboxController",
                    configuration.strLogDir + "InstrumentsSandboxController_$TIMESTAMP.log",
                    200 * 1024);

    _sandbox.setOutputStream(_outStream);
    _sandbox.addLogFileInfos("InstrumentsSandbox");

    return _sandbox.createSandbox(PLUGIN_INSTRUMENT, configuration, this);
}


/**************************** INSTRUMENTS MANAGEMENT **************************/

bool SandboxedInstrumentsSet::setInstrumentsFolder(const std::string& strPath)
{
    return _sandbox.setPluginsFolder(strPath);
}


int SandboxedInstrumentsSet::loadInstrumentPlugin(const std::string& strName)
{
    // Save the context (in case of crash)
    std::ostringstream str;
    str << "Method: loading" << endl;
	_strContext = str.str();

    return _sandbox.loadPlugin(strName);
}


bool SandboxedInstrumentsSet::setupInstrument(int instrument,
                                              const tExperimentParametersList& parameters)
{
    if (instrument >= _sandbox.nbPlugins())
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _currentInstrument = instrument;

    _outStream << "< SETUP " << " " << instrument << " " << parameters.size() << " ..." << endl;

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

    pChannel->startPacket(SANDBOX_COMMAND_INSTRUMENT_SETUP);
    pChannel->add(instrument);
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

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::createInstruments()
{
    // Save the context (in case of crash)
    std::ostringstream str;
    str << "Method: constructor" << endl;
	_strContext = str.str();
        
    return _sandbox.createPlugins();
}


unsigned int SandboxedInstrumentsSet::nbInstruments() const
{
    return _sandbox.nbPlugins();
}


int SandboxedInstrumentsSet::instrumentIndex(const std::string& strName) const
{
    return _sandbox.getPluginIndex(strName);
}


std::string SandboxedInstrumentsSet::instrumentName(int index) const
{
    return _sandbox.getPluginName(index);
}


/******************************** GENERAL EVENTS ******************************/

bool SandboxedInstrumentsSet::onExperimentDone()
{
    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_EXPERIMENT_DONE" << endl;

    // Save the context (in case of crash)
    std::ostringstream str;

    str << "Method: onExperimentDone" << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_EXPERIMENT_DONE);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


/************************** CLASSIFIER-RELATED EVENTS *************************/

bool SandboxedInstrumentsSet::onExperimentStarted(IClassifierInputSet* input_set)
{
    // Assertions
    assert(input_set);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_CLASSIFICATION_EXPERIMENT_STARTED " << input_set->id() << " " << input_set->isDoingDetection() << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onExperimentStarted" << endl
        << "Parameters:" << endl
        << "    - Number of images:     #" << input_set->nbImages() << endl
        << "    - Number of labels:     " << input_set->nbLabels() << endl
        << "    - Number of heuristics: " << input_set->nbHeuristics() << endl
        << "    - Number of features:   " << input_set->nbFeaturesTotal() << endl
        << "    - ROI extent:           " << input_set->roiExtent() << " pixels" << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_CLASSIFICATION_EXPERIMENT_STARTED);
    pChannel->add(input_set->id());
    pChannel->add(input_set->isDoingDetection());
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onClassifierTrainingStarted(IClassifierInputSet* input_set)
{
    // Assertions
    assert(input_set);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_CLASSIFIER_TRAINING_STARTED " << input_set->id() << " " << input_set->isDoingDetection() << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onClassifierTrainingStarted" << endl
        << "Parameters:" << endl
        << "    - Number of images:     #" << input_set->nbImages() << endl
        << "    - Number of labels:     " << input_set->nbLabels() << endl
        << "    - Number of heuristics: " << input_set->nbHeuristics() << endl
        << "    - Number of features:   " << input_set->nbFeaturesTotal() << endl
        << "    - ROI extent:           " << input_set->roiExtent() << " pixels" << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TRAINING_STARTED);
    pChannel->add(input_set->id());
    pChannel->add(input_set->isDoingDetection());
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onClassifierTrainingDone(IClassifierInputSet* input_set,
                                                       scalar_t train_error)
{
    // Assertions
    assert(input_set);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_CLASSIFIER_TRAINING_DONE " << train_error << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onClassifierTrainingDone" << endl
        << "Parameters:" << endl
        << "    - Number of images:     #" << input_set->nbImages() << endl
        << "    - Number of labels:     " << input_set->nbLabels() << endl
        << "    - Number of heuristics: " << input_set->nbHeuristics() << endl
        << "    - Number of features:   " << input_set->nbFeaturesTotal() << endl
        << "    - ROI extent:           " << input_set->roiExtent() << " pixels" << endl
        << "    - Train error:          " << train_error << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TRAINING_DONE);
    pChannel->add(train_error);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onClassifierTestStarted(IClassifierInputSet* input_set)
{
    // Assertions
    assert(input_set);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_CLASSIFIER_TEST_STARTED " << input_set->id() << " " << input_set->isDoingDetection() << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onClassifierTestStarted" << endl
        << "Parameters:" << endl
        << "    - Number of images:     #" << input_set->nbImages() << endl
        << "    - Number of labels:     " << input_set->nbLabels() << endl
        << "    - Number of heuristics: " << input_set->nbHeuristics() << endl
        << "    - Number of features:   " << input_set->nbFeaturesTotal() << endl
        << "    - ROI extent:           " << input_set->roiExtent() << " pixels" << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TEST_STARTED);
    pChannel->add(input_set->id());
    pChannel->add(input_set->isDoingDetection());
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onClassifierTestDone(IClassifierInputSet* input_set,
                                                   scalar_t test_error)
{
    // Assertions
    assert(input_set);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_CLASSIFIER_TEST_DONE " << test_error << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onClassifierTestDone" << endl
        << "Parameters:" << endl
        << "    - Number of images:     #" << input_set->nbImages() << endl
        << "    - Number of labels:     " << input_set->nbLabels() << endl
        << "    - Number of heuristics: " << input_set->nbHeuristics() << endl
        << "    - Number of features:   " << input_set->nbFeaturesTotal() << endl
        << "    - ROI extent:           " << input_set->roiExtent() << " pixels" << endl
        << "    - Test error:           " << test_error << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TEST_DONE);
    pChannel->add(test_error);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onClassifierClassificationDone(IClassifierInputSet* input_set,
                                                             unsigned int image,
                                                             unsigned int original_image,
                                                             const coordinates_t& position,
                                                             const Classifier::tClassificationResults& results,
                                                             Instrument::tClassificationError error)
{
    // Assertions
    assert(input_set);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_CLASSIFIER_CLASSIFICATION_DONE " << image << " "
               << original_image << " " << position.x << " " << position.y
               << " " << error << " " << results.size() << " ..." << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

    dim_t image_size = input_set->imageSize(image);

	str << "Method: onClassifierClassificationDone" << endl
        << "Parameters:" << endl
        << "    - Image:                #" << image << " (" << image_size.width << "x" << image_size.height << " pixels)" << endl
        << "    - Original image:       #" << original_image << endl
        << "    - ROI extent:           " << input_set->roiExtent() << " pixels" << endl
        << "    - ROI position:         (" << position.x << ", " << position.y << ")" << endl
        << "    - Number of labels:     " << input_set->nbLabels() << endl
        << "    - Number of heuristics: " << input_set->nbHeuristics() << endl
        << "    - Number of features:   " << input_set->nbFeaturesTotal() << endl
        << "    - Number of results:    " << results.size() << endl
        << "    - Error:                ";
    
    switch (error)
    {
        case Instrument::CLASSIFICATION_ERROR_NONE:                 str << "None"; break;
        case Instrument::CLASSIFICATION_ERROR_FALSE_ALARM:          str << "False alarm"; break;
        case Instrument::CLASSIFICATION_ERROR_FALSE_REJECTION:      str << "False rejection"; break;
        case Instrument::CLASSIFICATION_ERROR_WRONG_CLASSIFICATION: str << "Wrong classification"; break;
    }

    str << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_CLASSIFICATION_DONE);
    pChannel->add(image);
    pChannel->add(original_image);
    pChannel->add(position.x);
    pChannel->add(position.y);
    pChannel->add((unsigned int) error);
    pChannel->add((unsigned int) results.size());

    Classifier::tClassificationResults::const_iterator iter, iterEnd;
    for (iter = results.begin(), iterEnd = results.end(); iter != iterEnd; ++iter)
    {
        pChannel->add(iter->first);
        pChannel->add(iter->second);
    }

    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onFeaturesComputedByClassifier(bool detection,
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

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_FEATURES_COMPUTED_BY_CLASSIFIER " << detection << " "
               << training << " " << image << " " << original_image << " "
               << coords.x << " " << coords.y << " " << heuristic << " "
               << nbFeatures << " ..." << endl;

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onFeaturesComputedByClassifier" << endl
        << "Parameters:" << endl
        << "    - Detection:          " << detection << endl
        << "    - Training:           " << training << endl
        << "    - Image:              #" << image << endl
        << "    - Original image:     #" << original_image << endl
        << "    - ROI position:       (" << coords.x << ", " << coords.y << ")" << endl
        << "    - ROI extent:         " << roiExtent << endl
        << "    - Heuristic:          #" << heuristic << endl
        << "    - Number of features: " << nbFeatures << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_FEATURES_COMPUTED_BY_CLASSIFIER);
    pChannel->add(detection);
    pChannel->add(training);
    pChannel->add(image);
    pChannel->add(original_image);
    pChannel->add(coords.x);
    pChannel->add(coords.y);
    pChannel->add(roiExtent);
    pChannel->add(heuristic);
    pChannel->add(nbFeatures);
    pChannel->add((char*) indexes, nbFeatures * sizeof(unsigned int));
    pChannel->add((char*) values, nbFeatures * sizeof(scalar_t));
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


/************************* GOALPLANNER-RELATED EVENTS *************************/

bool SandboxedInstrumentsSet::onExperimentStarted(ITask* task)
{
    // Assertions
    assert(task);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_GOALPLANNING_EXPERIMENT_STARTED" << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(task, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onPlannerLearningStarted" << endl
        << "Parameters:" << endl
        << "    - Number of actions:    " << task->nbActions() << endl
        << "    - Number of views:      " << task->perception()->nbViews() << endl
        << "    - Number of heuristics: " << task->perception()->nbHeuristics() << endl
        << "    - Number of features:   " << task->perception()->nbFeaturesTotal() << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_GOALPLANNING_EXPERIMENT_STARTED);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pTaskProxy;
    _pTaskProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onPlannerLearningStarted(ITask* task)
{
    // Assertions
    assert(task);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_GOALPLANNER_LEARNING_STARTED" << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(task, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onPlannerLearningStarted" << endl
        << "Parameters:" << endl
        << "    - Number of actions:    " << task->nbActions() << endl
        << "    - Number of views:      " << task->perception()->nbViews() << endl
        << "    - Number of heuristics: " << task->perception()->nbHeuristics() << endl
        << "    - Number of features:   " << task->perception()->nbFeaturesTotal() << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_LEARNING_STARTED);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pTaskProxy;
    _pTaskProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onPlannerLearningDone(ITask* task, tResult result)
{
    // Assertions
    assert(task);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_GOALPLANNER_LEARNING_DONE " << result << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(task, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onPlannerLearningDone" << endl
        << "Parameters:" << endl
        << "    - Number of actions:    " << task->nbActions() << endl
        << "    - Number of views:      " << task->perception()->nbViews() << endl
        << "    - Number of heuristics: " << task->perception()->nbHeuristics() << endl
        << "    - Number of features:   " << task->perception()->nbFeaturesTotal() << endl
        << "    - Result:               ";

    switch (result)
    {
        case RESULT_NONE:           str << "None"; break;
        case RESULT_GOAL_REACHED:   str << "Goal reached"; break;
        case RESULT_TASK_FAILED:    str << "Task failed"; break;
    }
    
    str << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_LEARNING_DONE);
    pChannel->add((unsigned int) result);
    pChannel->sendPacket();

    // Wait the response
    bool res = pChannel->good();
    if (res)
        res = _sandbox.waitResponse();
    
    delete _pTaskProxy;
    _pTaskProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return res;
}


bool SandboxedInstrumentsSet::onPlannerTestStarted(ITask* task)
{
    // Assertions
    assert(task);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_GOALPLANNER_TEST_STARTED" << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(task, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onPlannerTestStarted" << endl
        << "Parameters:" << endl
        << "    - Number of actions:    " << task->nbActions() << endl
        << "    - Number of views:      " << task->perception()->nbViews() << endl
        << "    - Number of heuristics: " << task->perception()->nbHeuristics() << endl
        << "    - Number of features:   " << task->perception()->nbFeaturesTotal() << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_TEST_STARTED);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    delete _pTaskProxy;
    _pTaskProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


bool SandboxedInstrumentsSet::onPlannerTestDone(ITask* task, scalar_t score, tResult result)
{
    // Assertions
    assert(task);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_GOALPLANNER_TEST_DONE " << score << " " << result << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(task, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onPlannerTestDone" << endl
        << "Parameters:" << endl
        << "    - Number of actions:    " << task->nbActions() << endl
        << "    - Number of views:      " << task->perception()->nbViews() << endl
        << "    - Number of heuristics: " << task->perception()->nbHeuristics() << endl
        << "    - Number of features:   " << task->perception()->nbFeaturesTotal() << endl
        << "    - Score:                " << score << endl
        << "    - Result:               ";

    switch (result)
    {
        case RESULT_NONE:           str << "None"; break;
        case RESULT_GOAL_REACHED:   str << "Goal reached"; break;
        case RESULT_TASK_FAILED:    str << "Task failed"; break;
    }
    
    str << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_TEST_DONE);
    pChannel->add(score);
    pChannel->add((unsigned int) result);
    pChannel->sendPacket();

    // Wait the response
    bool res = pChannel->good();
    if (res)
        res = _sandbox.waitResponse();
    
    delete _pTaskProxy;
    _pTaskProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return res;
}


bool SandboxedInstrumentsSet::onPlannerActionChoosen(ITask* task, unsigned int action,
                                                     scalar_t reward, tResult result)
{
    // Assertions
    assert(task);

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_GOALPLANNER_ACTION_CHOOSEN " << action << " "
               << reward << " " << result << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pTaskProxy = new SandboxTaskProxy(task, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onPlannerActionChoosen" << endl
        << "Parameters:" << endl
        << "    - Number of actions:    " << task->nbActions() << endl
        << "    - Number of views:      " << task->perception()->nbViews() << endl
        << "    - Number of heuristics: " << task->perception()->nbHeuristics() << endl
        << "    - Number of features:   " << task->perception()->nbFeaturesTotal() << endl
        << "    - Action:               " << action << endl
        << "    - Reward:               " << reward << endl
        << "    - Result:               ";

    switch (result)
    {
        case RESULT_NONE:           str << "None"; break;
        case RESULT_GOAL_REACHED:   str << "Goal reached"; break;
        case RESULT_TASK_FAILED:    str << "Task failed"; break;
    }

    str << endl;

	_strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_ACTION_CHOOSEN);
    pChannel->add(action);
    pChannel->add(reward);
    pChannel->add((unsigned int) result);
    pChannel->sendPacket();

    // Wait the response
    bool res = pChannel->good();
    if (res)
        res = _sandbox.waitResponse();

    delete _pTaskProxy;
    _pTaskProxy = 0;

    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return res;
}


bool SandboxedInstrumentsSet::onFeaturesComputedByPlanner(unsigned int sequence,
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

    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_FEATURES_COMPUTED_BY_GOALPLANNER " << sequence << " " << view << " "
               << image << " " << coords.x << " " << coords.y << " " << heuristic
               << " " << nbFeatures << " ..." << endl;

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onFeaturesComputedByPlanner" << endl
        << "Parameters:" << endl
        << "    - Sequence:           #" << sequence << endl
        << "    - View:               #" << view << endl
        << "    - Image:              #" << image << endl
        << "    - ROI position:       (" << coords.x << ", " << coords.y << ")" << endl
        << "    - ROI extent:         " << roiExtent << endl
        << "    - Heuristic:          #" << heuristic << endl
        << "    - Number of features: " << nbFeatures << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_FEATURES_COMPUTED_BY_GOALPLANNER);
    pChannel->add(sequence);
    pChannel->add(view);
    pChannel->add(image);
    pChannel->add(coords.x);
    pChannel->add(coords.y);
    pChannel->add(roiExtent);
    pChannel->add(heuristic);
    pChannel->add(nbFeatures);
    pChannel->add((char*) indexes, nbFeatures * sizeof(unsigned int));
    pChannel->add((char*) values, nbFeatures * sizeof(scalar_t));
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


/************************** PREDICTOR-RELATED EVENTS **************************/

bool SandboxedInstrumentsSet::onFeatureListReported(const tFeatureList& features)
{
    if (_sandbox.nbPlugins() == 0)
        return true;

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< EVENT_FEATURE_LIST_REPORTED " << features.size() << " ..." << endl;

    // Save the context (in case of crash)
    std::ostringstream str;

	str << "Method: onFeatureListReported" << endl
        << "Parameters:" << endl
        << "    - Number of reported features: " << features.size() << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_EVENT_INSTRUMENTS_FEATURE_LIST_REPORTED);

    unsigned int nbFeatures = features.size();
    pChannel->add((unsigned int) features.size());

    tFeatureList::const_iterator iter, iterEnd;
    for (iter = features.begin(), iterEnd = features.end();
         pChannel->good() && (iter != iterEnd); ++iter)
    {
        pChannel->add(iter->heuristic);
        pChannel->add(iter->feature_index);
    }

    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    if (_lastError == ERROR_NONE)
        _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_INSTRUMENT_CRASHED : ERROR_NONE;

    return result;
}


/*********************************** METHODS **********************************/

tError SandboxedInstrumentsSet::getLastError()
{
    return (_lastError != ERROR_NONE ?
                _lastError :
                (_sandbox.getLastError() == ERROR_CHANNEL_SLAVE_CRASHED ?
                        ERROR_INSTRUMENT_CRASHED : _sandbox.getLastError()));
}


tCommandProcessingResult SandboxedInstrumentsSet::processResponse(tSandboxMessage message)
{
    CommunicationChannel* pChannel = _sandbox.channel();

    tCommandProcessingResult result = COMMAND_UNKNOWN;

    if (message == SANDBOX_MESSAGE_CURRENT_INSTRUMENT)
    {
        pChannel->read(&_currentInstrument);
        result = COMMAND_PROCESSED;
    } 

    else if (_pInputSetProxy)
    {
        result = _pInputSetProxy->processResponse(message);
    }

    if ((result == COMMAND_UNKNOWN) && _pTaskProxy)
        result = _pTaskProxy->processResponse(message);
    
    return result;
}
