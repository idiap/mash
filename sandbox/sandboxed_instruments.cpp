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


/** @file   sandboxed_instruments.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxedInstruments' class
*/

#include "sandboxed_instruments.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-sandboxing/declarations.h>
#include <mash-utils/errors.h>
#include <stdlib.h>
#include <assert.h>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include "dynlibs_manager_delegate.h"
#endif

using namespace std;
using namespace Mash;


/****************************** STATIC ATTRIBUTES *****************************/

SandboxedInstruments::tEventHandlersList SandboxedInstruments::handlers;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxedInstruments::SandboxedInstruments(const CommunicationChannel& channel,
                                           OutStream* pOutStream)
: ISandboxedObject(channel, pOutStream), _pManager(0),
  _inputSet(channel, pOutStream, 0, true), _task(channel, pOutStream, 0, true)
{
    if (handlers.empty())
    {
        handlers[SANDBOX_COMMAND_INSTRUMENT_SETUP]                              = &SandboxedInstruments::handleInstrumentSetupCommand;
        handlers[SANDBOX_EVENT_INSTRUMENTS_EXPERIMENT_DONE]                     = &SandboxedInstruments::handleExperimentDoneEvent;

        handlers[SANDBOX_EVENT_INSTRUMENTS_CLASSIFICATION_EXPERIMENT_STARTED]   = &SandboxedInstruments::handleClassificationExperimentStartedEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TRAINING_STARTED]         = &SandboxedInstruments::handleClassifierTrainingStartedEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TRAINING_DONE]            = &SandboxedInstruments::handleClassifierTrainingDoneEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TEST_STARTED]             = &SandboxedInstruments::handleClassifierTestStartedEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_TEST_DONE]                = &SandboxedInstruments::handleClassifierTestDoneEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_CLASSIFIER_CLASSIFICATION_DONE]      = &SandboxedInstruments::handleClassifierClassificationDoneEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_FEATURES_COMPUTED_BY_CLASSIFIER]     = &SandboxedInstruments::handleFeaturesComputedByClassifierEvent;
                                                                                
        handlers[SANDBOX_EVENT_INSTRUMENTS_GOALPLANNING_EXPERIMENT_STARTED]     = &SandboxedInstruments::handleGoalplanningExperimentStartedEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_LEARNING_STARTED]        = &SandboxedInstruments::handlePlannerLearningStartedEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_LEARNING_DONE]           = &SandboxedInstruments::handlePlannerLearningDoneEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_TEST_STARTED]            = &SandboxedInstruments::handlePlannerTestStartedEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_TEST_DONE]               = &SandboxedInstruments::handlePlannerTestDoneEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_GOALPLANNER_ACTION_CHOOSEN]          = &SandboxedInstruments::handlePlannerActionChoosenEvent;
        handlers[SANDBOX_EVENT_INSTRUMENTS_FEATURES_COMPUTED_BY_GOALPLANNER]    = &SandboxedInstruments::handleFeaturesComputedByPlannerEvent;
                                                                                
        handlers[SANDBOX_EVENT_INSTRUMENTS_FEATURE_LIST_REPORTED]               = &SandboxedInstruments::handleFeatureListReportedEvent;
    }
}


SandboxedInstruments::~SandboxedInstruments()
{
    // Destroy the instruments
    tInstrumentsIterator iter, iterEnd;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter)
    {
        DataWriter w(iter->pInstrument->writer);
        
        setWardenContext(&iter->wardenContext);
        delete iter->pInstrument;
        setWardenContext(0);
    }
    
    // Destroy the instruments manager
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    wardenEnableUnsafeFree();
#endif

    delete _pManager;
}


/******************** IMPLEMENTATION OF ISandboxedObject **********************/

tError SandboxedInstruments::setPluginsFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(!_pManager);
    
    _outStream << "Plugins folder: " << strPath << endl;
    
    // Create the heuristics manager
    _pManager = new InstrumentsManager(strPath);
    
    return ERROR_NONE;
}


tError SandboxedInstruments::loadPlugin(const std::string& strName)
{
    // Assertions
    assert(!strName.empty());
    assert(_pManager);

    _outStream << "Loading plugin '" << strName << "'" << endl;

    // Initialize the information structure about the instrument
    tInstrumentInfos infos;
    infos.pInstrument                               = 0;
    infos.strName                                   = strName;

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    infos.wardenContext.sandboxed_object            = 0;
    infos.wardenContext.memory_allocated            = 0;
    infos.wardenContext.memory_allocated_maximum    = 0;
    infos.wardenContext.memory_limit                = 200 * 1024 * 1024;
    infos.wardenContext.exceptions                  = WARDEN_EXCEPTION_DLOPEN;

    DynlibsManagerDelegate delegate(&infos.wardenContext);
    _pManager->setDelegate(&delegate);
#endif
    
    DYNLIB_HANDLE handle = _pManager->loadDynamicLibrary(strName);

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _pManager->setDelegate(0);
    infos.wardenContext.exceptions = 0;
#endif

    if (!handle)
    {
        string desc = _pManager->getLastErrorDescription();
        if (desc.empty())
            desc = getErrorDescription(_pManager->getLastError());
        
        _outStream << desc << endl;
        return _pManager->getLastError();
    }
    
    // Add it to the list of instruments
    _instruments.push_back(infos);
    
    return ERROR_NONE;
}


tError SandboxedInstruments::createPlugins(Mash::OutStream* pOutStream,
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
    assert(!_instruments.empty());
    assert(!dataWriters.empty());
    assert(dataWriters.size() == _instruments.size());

    _outStream << "Creation of the objects defined in the plugins..." << endl;

    _channel.startPacket(SANDBOX_MESSAGE_KEEP_ALIVE);
    _channel.sendPacket();

    // Create the instruments
    tInstrumentsIterator iter, iterEnd;
    vector<DataWriter>::const_iterator iter2;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(), iter2 = dataWriters.begin();
         iter != iterEnd; ++iter, ++iter2, ++index)
    {
        _outStream << "--- Creation of '" << iter->strName << "'" << endl;

        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
        DynlibsManagerDelegate delegate(&iter->wardenContext);
        _pManager->setDelegate(&delegate);
#endif

        iter->pInstrument = _pManager->create(iter->strName);

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
        _pManager->setDelegate(0);
#endif

        if (!iter->pInstrument)
        {
            _outStream << getErrorDescription(_pManager->getLastError()) << endl;
            return _pManager->getLastError();
        }

        iter->pInstrument->writer = *iter2;

        _channel.startPacket(SANDBOX_MESSAGE_KEEP_ALIVE);
        _channel.sendPacket();
    }    
    
    return ERROR_NONE;
}


void SandboxedInstruments::handleCommand(tSandboxMessage command)
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    assert(!handlers.empty());

    tEventHandlersIterator iter = handlers.find(command);
    if (iter != handlers.end())
    {
        tEventHandler handler = iter->second;
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


/**************************** EVENTS HANDLERS ********************************/

tError SandboxedInstruments::handleInstrumentSetupCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the parameters
    unsigned int instrument = 0;
    unsigned int nbParameters = 0;

    _channel.read(&instrument);
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

    _outStream << "> SETUP " << instrument << " " << nbParameters << " ..." << endl;
    
    if (instrument >= _instruments.size())
    {
        _outStream << getErrorDescription(ERROR_INSTRUMENT_SETUP_FAILED) << endl;
        return ERROR_INSTRUMENT_SETUP_FAILED;
    }
    
    // Tell the instrument to setup itself using the parameters
    setWardenContext(&_instruments[instrument].wardenContext);
    _instruments[instrument].pInstrument->setup(parameters);
    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleExperimentDoneEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    _outStream << "> EVENT_EXPERIMENT_DONE" << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onExperimentDone();
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleClassificationExperimentStartedEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the id of the input set
    unsigned int id;
    if (!_channel.read(&id))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    // Retrieve the type of task
    bool detection;
    if (!_channel.read(&detection))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> EVENT_CLASSIFICATION_EXPERIMENT_STARTED " << id << " " << detection << endl;
    
    // Change the id of the input set if necessary
    if (_inputSet.id() != id)
       _inputSet.setup(id, detection);
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onExperimentStarted(&_inputSet);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleClassifierTrainingStartedEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the id of the input set
    unsigned int id;
    if (!_channel.read(&id))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    // Retrieve the type of task
    bool detection;
    if (!_channel.read(&detection))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> EVENT_CLASSIFIER_TRAINING_STARTED " << id << " " << detection << endl;
    
    // Change the id of the input set if necessary
    if (_inputSet.id() != id)
       _inputSet.setup(id, detection);
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onClassifierTrainingStarted(&_inputSet);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleClassifierTrainingDoneEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the train error
    scalar_t train_error;
    if (!_channel.read(&train_error))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> EVENT_CLASSIFIER_TRAINING_DONE " << train_error << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onClassifierTrainingDone(&_inputSet, train_error);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleClassifierTestStartedEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the id of the input set
    unsigned int id;
    if (!_channel.read(&id))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    // Retrieve the type of task
    bool detection;
    if (!_channel.read(&detection))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> EVENT_CLASSIFIER_TEST_STARTED " << id << " " << detection << endl;
    
    // Change the id of the input set if necessary
    if (_inputSet.id() != id)
       _inputSet.setup(id, detection);
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onClassifierTestStarted(&_inputSet);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleClassifierTestDoneEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the train error
    scalar_t test_error;
    if (!_channel.read(&test_error))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> EVENT_CLASSIFIER_TEST_DONE " << test_error << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onClassifierTestDone(&_inputSet, test_error);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleClassifierClassificationDoneEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the parameters
    unsigned int                        image;
    unsigned int                        original_image;
    coordinates_t                       position;
    unsigned int                        error;
    unsigned int                        nbResults;
    Classifier::tClassificationResults  results;

    if (!_channel.read(&image) ||
        !_channel.read(&original_image) ||
        !_channel.read(&position.x) ||
        !_channel.read(&position.y) ||
        !_channel.read(&error) ||
        !_channel.read(&nbResults))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    for (unsigned int i = 0; i < nbResults; ++i)
    {
        int         label;
        scalar_t    score;

        if (!_channel.read(&label) || !_channel.read(&score))
        {
            _outStream << getErrorDescription(_channel.getLastError()) << endl;
            return _channel.getLastError();
        }
        
        results[label] = score;
    }

    _outStream << "> EVENT_CLASSIFIER_CLASSIFICATION_DONE " << image << " "
               << original_image << " " << position.x << " " << position.y
               << " " << error << " " << nbResults << " ..." << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onClassifierClassificationDone(&_inputSet, image, original_image, position, results,
                                                          (Instrument::tClassificationError) error);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleFeaturesComputedByClassifierEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the parameters
    bool            detection;
    bool            training;
    unsigned int    image;
    unsigned int    original_image;
    coordinates_t   coords;
    unsigned int    roiExtent;
    unsigned int    heuristic;
    unsigned int    nbFeatures;
    unsigned int*   indexes = 0;
    scalar_t*       values = 0;
                                    
    if (!_channel.read(&detection) ||
        !_channel.read(&training) ||
        !_channel.read(&image) ||
        !_channel.read(&original_image) ||
        !_channel.read(&coords.x) ||
        !_channel.read(&coords.y) ||
        !_channel.read(&roiExtent) ||
        !_channel.read(&heuristic) ||
        !_channel.read(&nbFeatures))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    indexes = new unsigned int[nbFeatures];
    values = new scalar_t[nbFeatures];

    _channel.read((char*) indexes, nbFeatures * sizeof(unsigned int));
    _channel.read((char*) values, nbFeatures * sizeof(scalar_t));

    if (!_channel.good())
    {
        delete[] indexes;
        delete[] values;

        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> EVENT_FEATURES_COMPUTED_BY_CLASSIFIER " << detection << " "
               << training << " " << image << " " << original_image << " "
               << coords.x << " " << coords.y << " " << roiExtent << " "
               << heuristic << " " << nbFeatures << " ..." << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onFeaturesComputedByClassifier(detection, training, image,
                                                          original_image, coords, roiExtent,
                                                          heuristic, nbFeatures, indexes, values);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    delete[] indexes;
    delete[] values;

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleGoalplanningExperimentStartedEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    _outStream << "> EVENT_GOALPLANNING_EXPERIMENT_STARTED" << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onExperimentStarted(&_task);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handlePlannerLearningStartedEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    _outStream << "> EVENT_GOALPLANNER_LEARNING_STARTED" << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onPlannerLearningStarted(&_task);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handlePlannerLearningDoneEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the parameters
    unsigned int intResult;
    tResult result;
    
    if (!_channel.read(&intResult))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    result = (tResult) intResult;

    _outStream << "> EVENT_GOALPLANNER_LEARNING_DONE " << result << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onPlannerLearningDone(&_task, result);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handlePlannerTestStartedEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    _outStream << "> EVENT_GOALPLANNER_TEST_STARTED" << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onPlannerTestStarted(&_task);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handlePlannerTestDoneEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the parameters
    scalar_t        score;
    unsigned int    intResult;
    tResult         result;
    
    if (!_channel.read(&score) ||
        !_channel.read(&intResult))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    result = (tResult) intResult;

    _outStream << "> EVENT_GOALPLANNER_TEST_DONE " << score << " " << result << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onPlannerTestDone(&_task, score, result);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handlePlannerActionChoosenEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the parameters
    unsigned int    action;
    scalar_t        reward;
    unsigned int    intResult;
    tResult         result;
    
    if (!_channel.read(&action) ||
        !_channel.read(&reward) ||
        !_channel.read(&intResult))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    result = (tResult) intResult;

    _outStream << "> EVENT_GOALPLANNER_ACTION_CHOOSEN " << action << " "
               << reward << " " << result << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onPlannerActionChoosen(&_task, action, reward, result);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleFeaturesComputedByPlannerEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the parameters
    unsigned int    sequence;
    unsigned int    view;
    unsigned int    image;
    coordinates_t   coords;
    unsigned int    roiExtent;
    unsigned int    heuristic;
    unsigned int    nbFeatures;
    unsigned int*   indexes = 0;
    scalar_t*       values = 0;
                                    
    if (!_channel.read(&sequence) ||
        !_channel.read(&view) ||
        !_channel.read(&image) ||
        !_channel.read(&coords.x) ||
        !_channel.read(&coords.y) ||
        !_channel.read(&roiExtent) ||
        !_channel.read(&heuristic) ||
        !_channel.read(&nbFeatures))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    indexes = new unsigned int[nbFeatures];
    values = new scalar_t[nbFeatures];

    _channel.read((char*) indexes, nbFeatures * sizeof(unsigned int));
    _channel.read((char*) values, nbFeatures * sizeof(scalar_t));

    if (!_channel.good())
    {
        delete[] indexes;
        delete[] values;

        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> EVENT_FEATURES_COMPUTED_BY_GOALPLANNER " << sequence << " " << view << " "
               << image << " " << coords.x << " " << coords.y << " " << roiExtent
               << " " << heuristic << " " << nbFeatures << " ..." << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onFeaturesComputedByPlanner(sequence, view, image, coords, roiExtent,
                                                       heuristic, nbFeatures, indexes, values);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    delete[] indexes;
    delete[] values;

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedInstruments::handleFeatureListReportedEvent()
{
    // Assertions
    assert(_pManager);
    assert(!_instruments.empty());
    
    // Retrieve the parameters
    unsigned int nbFeatures = 0;
    _channel.read(&nbFeatures);
    
    tFeatureList list;
    for (unsigned int i = 0; _channel.good() && (i < nbFeatures); ++i)
    {
        unsigned int heuristic;
        unsigned int feature;
        
        if (_channel.read(&heuristic) && _channel.read(&feature))
            list.push_back(tFeature(heuristic, feature));
    }

    if (!_channel.good())
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> EVENT_FEATURE_LIST_REPORTED " << nbFeatures << " ..." << endl;
    
    // Forward the event to all the instruments
    tInstrumentsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _instruments.begin(), iterEnd = _instruments.end(); iter != iterEnd; ++iter, ++index)
    {
        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_INSTRUMENT);
        _channel.add(index);
        _channel.sendPacket();

        setWardenContext(&iter->wardenContext);
        iter->pInstrument->onFeatureListReported(list);
        setWardenContext(0);
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}
