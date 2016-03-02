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


/** @file   trusted_classifier.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'TrustedClassifier' class
*/

#include "trusted_classifier.h"
#include <mash-classification/classifier_input_set.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <sstream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

TrustedClassifier::TrustedClassifier()
: _pManager(0), _pClassifier(0), _lastError(ERROR_NONE)
{
    _outStream.setVerbosityLevel(3);
}


TrustedClassifier::~TrustedClassifier()
{
    delete _pClassifier;
    delete _pManager;

    _outStream.deleteFile();
}


/********************************* MANAGEMENT *********************************/

void TrustedClassifier::configure(const std::string& strLogFolder,
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

bool TrustedClassifier::setClassifiersFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(!_pManager);
    assert(!_pClassifier);
    
    _outStream << "Classifiers folder: " << strPath << endl;
    
    // Create the classifiers manager
    _pManager = new ClassifiersManager(strPath);
    
    return true;
}


bool TrustedClassifier::loadClassifierPlugin(const std::string& strName,
                                             const std::string& strModelFile,
                                             const std::string& strInternalDataFile)
{
    // Assertions
    assert(!strName.empty());
    assert(_pManager);
    assert(!_pClassifier);

    _outStream << "Loading classifier '" << strName << "'" << endl;
    
    
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
    _pClassifier = _pManager->create(strName);

    if (!_pClassifier)
    {
        _outStream << getErrorDescription(_pManager->getLastError()) << endl;
        return false;
    }

    _pClassifier->outStream.setVerbosityLevel(1);
    _pClassifier->outStream.open("Predictor", _strLogFolder + "Predictor_$TIMESTAMP.log");

    _pClassifier->writer.open(_strReportFolder + "predictor.data");
    
    
    // Load the input model
    if (!strModelFile.empty())
    {
        if (_inModel.open(strModelFile) && !strInternalDataFile.empty())
            _inInternalData.open(strInternalDataFile);
    }


    // Setup the output model
    _outModel.create(_strReportFolder + "predictor.model");
    _pClassifier->outInternalData.open(_strReportFolder + "predictor.internal");

    // Setup the cache
    _pClassifier->outFeatureCache.open("predictor.cache");
    _pClassifier->inFeatureCache.open("predictor.cache");
    
    return true;
}


void TrustedClassifier::setNotifier(INotifier* pNotifier)
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
    
    _pClassifier->notifier.useNotifier(pNotifier);
}


/*********************************** METHODS **********************************/

bool TrustedClassifier::setSeed(unsigned int seed)
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);
    
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> SET_SEED " << seed << endl;
    
    // Set the seed of the classifier
    _pClassifier->generator.setSeed(seed);

    return true;
}


bool TrustedClassifier::setup(const tExperimentParametersList& parameters)
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> SETUP " << parameters.size() << " ..." << endl;
    
    // Tell the classifier to setup itself using the parameters
    if (!_pClassifier->setup(parameters))
    {
        _lastError = ERROR_CLASSIFIER_SETUP_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


bool TrustedClassifier::loadModel(IClassifierInputSet* input_set)
{
    // Assertions
    assert(input_set);
    assert(_pManager);
    assert(_pClassifier);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> LOAD_MODEL " << _inModel.getFileName() << endl;

    if (!_inModel.isReadable())
    {
        _lastError = ERROR_CLASSIFIER_MODEL_LOADING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    // Set the seed of the classifier
    _pClassifier->generator.setSeed(_inModel.predictorSeed());

    // Inform the model about the available heuristics
    for (unsigned int i = 0; i < input_set->nbHeuristics(); ++i)
        _inModel.addHeuristic(i, input_set->heuristicName(i));

    if (!_inModel.lockHeuristics())
    {
        _lastError = ERROR_CLASSIFIER_MODEL_MISSING_HEURISTIC;
        _outStream << getErrorDescription(_lastError) << endl;

        tStringList missing = _inModel.missingHeuristics();
        for (unsigned int i = 0; i < missing.size(); ++i)
            _outStream << "    - " << missing[i] << endl;

        return false;
    }

    // Inform the Input Set about the heuristics that are already in the model
    ClassifierInputSet* pInputSet = dynamic_cast<ClassifierInputSet*>(input_set);
    if (pInputSet)
    {
        for (unsigned int i = 0; i < _inModel.nbHeuristics(); ++i)
            pInputSet->markHeuristicAsUsedByModel(_inModel.fromModel(i));
    }

    // Tell the classifier to load the model
    if (!_pClassifier->loadModel(_inModel, _inInternalData))
    {
        _lastError = ERROR_CLASSIFIER_MODEL_LOADING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


bool TrustedClassifier::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    // Assertions
    assert(input_set);
    assert(_pManager);
    assert(_pClassifier);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> TRAIN " << input_set->id() << " " << input_set->isDoingDetection() << endl;

    // Tell the classifier to train itself
    if (!_pClassifier->train(input_set, train_error))
    {
        _lastError = ERROR_CLASSIFIER_TRAINING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


bool TrustedClassifier::classify(IClassifierInputSet* input_set,
                                   unsigned int image,
                                   const coordinates_t& position,
                                   Classifier::tClassificationResults &results)
{
    // Assertions
    assert(input_set);
    assert(_pManager);
    assert(_pClassifier);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> CLASSIFY " << " " << input_set->id() << " "
               << input_set->isDoingDetection() << " " << image << " "
               << position.x << " " << position.y << endl;

    // Tell the classifier to classify the image
    if (!_pClassifier->classify(input_set, image, position, results))
    {
        _lastError = ERROR_CLASSIFIER_CLASSIFICATION_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


bool TrustedClassifier::reportFeaturesUsed(IClassifierInputSet* input_set,
                                           tFeatureList &list)
{
    // Assertions
    assert(input_set);
    assert(_pManager);
    assert(_pClassifier);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "> REPORT_FEATURES_USED" << endl;

    // Ask the classifier about the features used
    if (!_pClassifier->reportFeaturesUsed(list))
    {
        _lastError = ERROR_CLASSIFIER_REPORTING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }
    
    // Inform the model about the used heuristics
    if (_outModel.isWritable())
    {
        _outModel.setPredictorSeed(_pClassifier->generator.seed());
        
        tFeatureList::iterator iter, iterEnd;
        for (iter = list.begin(), iterEnd = list.end(); iter != iterEnd; ++iter)
        {
            _outModel.addHeuristic(iter->heuristic, input_set->heuristicName(iter->heuristic),
                                   input_set->heuristicSeed(iter->heuristic));
        }
        
        _outModel.lockHeuristics();
    }
        
    return true;
}


bool TrustedClassifier::saveModel()
{
    // Assertions
    assert(_pManager);
    assert(_pClassifier);

    if (getLastError() != ERROR_NONE)
    {
        _lastError = ERROR_CLASSIFIER_MODEL_SAVING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    _outStream << "> SAVE_MODEL" << endl;

    if (!_outModel.isWritable())
        return false;

    // Tell the classifier to save the model
    if (!_pClassifier->saveModel(_outModel))
    {
        _lastError = ERROR_CLASSIFIER_MODEL_SAVING_FAILED;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }

    return true;
}


tError TrustedClassifier::getLastError()
{
    return (((_lastError != ERROR_NONE) || !_pManager) ? _lastError : _pManager->getLastError());
}
