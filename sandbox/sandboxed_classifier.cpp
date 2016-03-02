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


/** @file   sandboxed_classifier.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxedClassifier' class
*/

#include "sandboxed_classifier.h"
#include <mash-utils/errors.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include "dynlibs_manager_delegate.h"
#endif


using namespace std;
using namespace Mash;


/****************************** STATIC ATTRIBUTES *****************************/

SandboxedClassifier::tCommandHandlersList SandboxedClassifier::handlers;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxedClassifier::SandboxedClassifier(const CommunicationChannel& channel,
                                         OutStream* pOutStream)
: ISandboxedObject(channel, pOutStream), _pManager(0), _pClassifier(0),
  _inputSet(channel, pOutStream, &_wardenContext),
  _notifier(channel, pOutStream, &_wardenContext)
{
    if (handlers.empty())
    {
        handlers[SANDBOX_COMMAND_CLASSIFIER_SET_SEED]               = &SandboxedClassifier::handleSetSeedCommand;
        handlers[SANDBOX_COMMAND_CLASSIFIER_SETUP]                  = &SandboxedClassifier::handleSetupCommand;
        handlers[SANDBOX_COMMAND_LOAD_MODEL]                        = &SandboxedClassifier::handleLoadModelCommand;
        handlers[SANDBOX_COMMAND_CLASSIFIER_TRAIN]                  = &SandboxedClassifier::handleTrainCommand;
        handlers[SANDBOX_COMMAND_CLASSIFIER_CLASSIFY]               = &SandboxedClassifier::handleClassifyCommand;
        handlers[SANDBOX_COMMAND_CLASSIFIER_REPORT_FEATURES_USED]   = &SandboxedClassifier::handleReportFeaturesUsedCommand;
        handlers[SANDBOX_COMMAND_SAVE_MODEL]                        = &SandboxedClassifier::handleSaveModelCommand;
    }

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _wardenContext.sandboxed_object            = 0;
    _wardenContext.memory_allocated            = 0;
    _wardenContext.memory_allocated_maximum    = 0;
    _wardenContext.memory_limit                = 2LL * 1024 * 1024 * 1024;
#endif
}


SandboxedClassifier::~SandboxedClassifier()
{
    // Destroy the classifier
    OutStream s(_pClassifier->outStream);
    DataWriter w1(_pClassifier->outFeatureCache);
    DataReader r1(_pClassifier->inFeatureCache);
    DataWriter w2(_pClassifier->writer);
    DataWriter w3(_pClassifier->outInternalData);
    
    setWardenContext(&_wardenContext);
    delete _pClassifier;
    setWardenContext(0);

    // Destroy the classifiers manager
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    wardenEnableUnsafeFree();
#endif
    delete _pManager;
}


/******************** IMPLEMENTATION OF ISandboxedObject **********************/

tError SandboxedClassifier::setPluginsFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(!_pManager);
    assert(!_pClassifier);
    
    _outStream << "Plugins folder: " << strPath << endl;
    
    // Create the classifiers manager
    _pManager = new ClassifiersManager(strPath);
    
    return ERROR_NONE;
}


tError SandboxedClassifier::loadPlugin(const std::string& strName)
{
    // Assertions
    assert(!strName.empty());
    assert(_pManager);
    assert(!_pClassifier);

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
    
    // Save the name of the classifier for later (needed by createPlugins())
    _strClassifierName = strName;
    
    return ERROR_NONE;
}


tError SandboxedClassifier::createPlugins(Mash::OutStream* pOutStream,
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
    assert(!_pClassifier);
    assert(!_strClassifierName.empty());

    _outStream << "Creation of the objects defined in the plugins..." << endl;

    // Create the classifier
    _outStream << "--- Creation of '" << _strClassifierName << "'" << endl;

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    DynlibsManagerDelegate delegate(&_wardenContext);
    _pManager->setDelegate(&delegate);
#endif

    _pClassifier = _pManager->create(_strClassifierName);

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _pManager->setDelegate(0);
#endif

    if (!_pClassifier)
    {
        _outStream << getErrorDescription(_pManager->getLastError()) << endl;
        return _pManager->getLastError();
    }

    if (pOutStream)
        _pClassifier->outStream = *pOutStream;

    if (!dataWriters.empty())
        _pClassifier->writer = dataWriters[0];

    if (!outCache.empty())
        _pClassifier->outFeatureCache = outCache[0];

    if (!inCache.empty())
        _pClassifier->inFeatureCache = inCache[0];

    _pClassifier->notifier.useNotifier(&_notifier);

    _inModel                        = inModel;
    _inInternalData                 = inInternalData;
    _outModel                       = outModel;
    _pClassifier->outInternalData   = outInternalData;
    
    return ERROR_NONE;
}


void SandboxedClassifier::handleCommand(tSandboxMessage command)
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
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

tError SandboxedClassifier::handleSetSeedCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
    
    // Retrieve the seed
    unsigned int seed;
    if (!_channel.read(&seed))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> SET_SEED " << seed << endl;
    
    // Set the seed of the classifier
    _pClassifier->generator.setSeed(seed);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedClassifier::handleSetupCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
    
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
    if (!_pClassifier->setup(parameters))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_CLASSIFIER_SETUP_FAILED) << endl;
        return ERROR_CLASSIFIER_SETUP_FAILED;
    }

    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedClassifier::handleLoadModelCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
    
    _outStream << "> LOAD_MODEL" << endl;
    
    if (!_inModel.isReadable())
    {
        _outStream << getErrorDescription(ERROR_CLASSIFIER_MODEL_LOADING_FAILED) << endl;
        return ERROR_CLASSIFIER_MODEL_LOADING_FAILED;
    }
    
    // Inform the model about the available heuristics
    for (unsigned int i = 0; i < _inputSet.nbHeuristics(); ++i)
        _inModel.addHeuristic(i, _inputSet.heuristicName(i));
    
    if (!_inModel.lockHeuristics())
    {
        _outStream << getErrorDescription(ERROR_CLASSIFIER_MODEL_MISSING_HEURISTIC) << endl;

        tStringList missing = _inModel.missingHeuristics();
        for (unsigned int i = 0; i < missing.size(); ++i)
            _outStream << "    - " << missing[i] << endl;

        return ERROR_CLASSIFIER_MODEL_MISSING_HEURISTIC;
    }

    // Inform the Input Set about the heuristics that are already in the model
    for (unsigned int i = 0; i < _inModel.nbHeuristics(); ++i)
        _inputSet.markHeuristicAsUsedByModel(_inModel.fromModel(i));
    
    // Tell the classifier to load the model
    setWardenContext(&_wardenContext);
    if (!_pClassifier->loadModel(_inModel, _inInternalData))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_CLASSIFIER_MODEL_LOADING_FAILED) << endl;
        return ERROR_CLASSIFIER_MODEL_LOADING_FAILED;
    }
    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedClassifier::handleTrainCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
    
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

    _outStream << "> TRAIN " << id << " " << detection << endl;
    
    // Change the id of the input set if necessary
    if (_inputSet.id() != id)
       _inputSet.setup(id, detection);
    
    // Tell the classifier to train itself
    setWardenContext(&_wardenContext);
    scalar_t train_error = 0 * log(0);
    if (!_pClassifier->train(&_inputSet, train_error))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_CLASSIFIER_TRAINING_FAILED) << endl;
        return ERROR_CLASSIFIER_TRAINING_FAILED;
    }
    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    
    if (!isnan(train_error))
        _channel.add(train_error);
    
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedClassifier::handleClassifyCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
    
    // Retrieve the id of the input set
    unsigned int id;
    _channel.read(&id);

    // Retrieve the type of task
    bool detection;
    _channel.read(&detection);

    // Retrieve the image index
    unsigned int image;
    _channel.read(&image);

    // Retrieve the coordinates index
    coordinates_t position;
    _channel.read(&position.x);
    _channel.read(&position.y);
    
    if (!_channel.good())
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> CLASSIFY " << id << " " << detection << " " << image
               << " " << position.x << " " << position.y << endl;

    // Change the id of the input set if necessary
    if (_inputSet.id() != id)
        _inputSet.setup(id, detection);
    
    // Tell the classifier to classify the image
    setWardenContext(&_wardenContext);
    Classifier::tClassificationResults* results = new Classifier::tClassificationResults();
    if (!_pClassifier->classify(&_inputSet, image, position, *results))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_CLASSIFIER_CLASSIFICATION_FAILED) << endl;
        return ERROR_CLASSIFIER_CLASSIFICATION_FAILED;
    }
    setWardenContext(0);

    // Send the results
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add((unsigned int) results->size());
    
    Classifier::tClassificationResults::iterator iter, iterEnd;
    for (iter = results->begin(), iterEnd = results->end();
         _channel.good() && (iter != iterEnd); ++iter)
    {
        _channel.add(iter->first);
        _channel.add(iter->second);
    }

    _channel.sendPacket();

    setWardenContext(&_wardenContext);
    delete results;
    setWardenContext(0);

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedClassifier::handleReportFeaturesUsedCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);

    _outStream << "> REPORT_FEATURES_USED" << endl;

    // Ask the classifier about the features used
    setWardenContext(&_wardenContext);
    tFeatureList* features = new tFeatureList();
    if (!_pClassifier->reportFeaturesUsed(*features))
    {
        delete features;
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_CLASSIFIER_REPORTING_FAILED) << endl;
        return ERROR_CLASSIFIER_REPORTING_FAILED;
    }
    setWardenContext(0);

    // Inform the model about the used heuristics
    tFeatureList::iterator iter, iterEnd;
    if (_outModel.isWritable())
    {
        _outModel.setPredictorSeed(_pClassifier->generator.seed());
        
        for (iter = features->begin(), iterEnd = features->end(); iter != iterEnd; ++iter)
        {
            _outModel.addHeuristic(iter->heuristic, _inputSet.heuristicName(iter->heuristic),
                                   _inputSet.heuristicSeed(iter->heuristic));
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


tError SandboxedClassifier::handleSaveModelCommand()
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
    
    _outStream << "> SAVE_MODEL" << endl;
    
    if (!_outModel.isWritable())
    {
        _outStream << getErrorDescription(ERROR_CLASSIFIER_MODEL_SAVING_FAILED) << endl;
        return ERROR_CLASSIFIER_MODEL_SAVING_FAILED;
    }
    
    // Tell the classifier to save the model
    setWardenContext(&_wardenContext);
    if (!_pClassifier->saveModel(_outModel))
    {
        setWardenContext(0);
        _outStream << getErrorDescription(ERROR_CLASSIFIER_MODEL_SAVING_FAILED) << endl;
        return ERROR_CLASSIFIER_MODEL_SAVING_FAILED;
    }
    setWardenContext(0);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}
