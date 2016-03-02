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


/** @file   classification_task.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'ClassificationTask' class
*/

#include "classification_task.h"
#include "listener.h"
#include <mash-utils/random_number_generator.h>
#include <mash-utils/errors.h>
#include <mash-classification/object_intersecter.h>
#include <mash/sandboxed_heuristics_set.h>
#include <mash/trusted_heuristics_set.h>
#include <mash-classification/sandboxed_classifier.h>
#include <mash-classification/trusted_classifier.h>
#include <mash-instrumentation/sandboxed_instruments_set.h>
#include <mash-instrumentation/trusted_instruments_set.h>
#include <vector>
#include <algorithm>
#include <limits>
#include <numeric>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>


using namespace std;
using namespace Mash;


/****************************** UTILITY FUNCTIONS *****************************/

bool value_comparer(Classifier::tClassificationResults::value_type &i1,
                    Classifier::tClassificationResults::value_type &i2)
{
    return i1.second < i2.second;
}


/************************* CONSTRUCTION / DESTRUCTION *************************/

ClassificationTask::ClassificationTask(Listener* pListener, bool detection)
: TaskController(pListener), _pClassifierDelegate(0), _inputSet(200, detection),
  _bUseModel(false)
{
    setGlobalSeed(time(0));
}


ClassificationTask::~ClassificationTask()
{
    delete _pClassifierDelegate;
}


/********************** IMPLEMENTATION OF TaskController **********************/

tError ClassificationTask::setup(const tTaskControllerConfiguration& configuration)
{
    // Assertions
    assert(!_pClassifierDelegate);
    assert(!_pInstrumentsSet);

    // Creates the Heuristics Set
    if (configuration.heuristicsSandboxConfiguration)
    {
        SandboxedHeuristicsSet* pHeuristicsSet = new SandboxedHeuristicsSet();
        _inputSet.featuresComputer()->setHeuristicsSet(pHeuristicsSet);

        if (!pHeuristicsSet->createSandbox(*configuration.heuristicsSandboxConfiguration))
            return pHeuristicsSet->getLastError();
    }
    else
    {
        TrustedHeuristicsSet* pHeuristicsSet = new TrustedHeuristicsSet();
        _inputSet.featuresComputer()->setHeuristicsSet(pHeuristicsSet);

        pHeuristicsSet->configure(configuration.strLogFolder);
    }

    if (!getFeaturesComputer()->heuristicsSet()->setHeuristicsFolder(configuration.strHeuristicsFolder))
        return getFeaturesComputer()->getLastError();

    // Creates the Classifier Delegate
    if (configuration.predictorSandboxConfiguration)
    {
        SandboxedClassifier* pDelegate = new SandboxedClassifier();
        _pClassifierDelegate = pDelegate;

        if (!pDelegate->createSandbox(*configuration.predictorSandboxConfiguration))
            return pDelegate->getLastError();
    }
    else
    {
        TrustedClassifier* pDelegate = new TrustedClassifier();
        _pClassifierDelegate = pDelegate;

        pDelegate->configure(configuration.strLogFolder, configuration.strReportFolder);
    }
    
    if (!_pClassifierDelegate->setClassifiersFolder(configuration.strPredictorsFolder))
        return _pClassifierDelegate->getLastError();

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

    // Inform the Input Set about its listener
    _listener.pInstrumentsSet = _pInstrumentsSet;
    _inputSet.setListener(&_listener);

    return ERROR_NONE;
}


void ClassificationTask::setGlobalSeed(unsigned int seed)
{
    RandomNumberGenerator globalGenerator;

    globalGenerator.setSeed(seed);

    for (unsigned int i = 0; i < COUNT_SEEDS; ++i)
        _seeds[i] = globalGenerator.randomize();

    getFeaturesComputer()->setSeed(_seeds[SEED_HEURISTICS]);
}


Mash::tError ClassificationTask::setClient(Mash::Client* pClient)
{
    assert(pClient);

    return _inputSet.setClient(pClient);
}


Mash::Client* ClassificationTask::getClient()
{
    return _inputSet.getClient();
}


ClassificationTask::tResult ClassificationTask::setParameters(const Mash::tExperimentParametersList& parameters)
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

    Mash::tError error = _inputSet.setParameters(parameters, _seeds[SEED_IMAGES]);
    if (error != ERROR_NONE)
        return tResult(error, _inputSet.getLastExperimentParameterError());

    return tResult();
}


ClassificationTask::tResult ClassificationTask::loadPredictor(const std::string strName,
                                                              const std::string& strModelFile,
                                                              const std::string& strInternalDataFile,
                                                              unsigned int seed)
{
    // Assertions
    assert(_pClassifierDelegate);

    if (!_pClassifierDelegate->loadClassifierPlugin(strName, strModelFile, strInternalDataFile))
    {
        if (_pClassifierDelegate->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED)
            return tResult(ERROR_CLASSIFIER_CRASHED);
        else
            return tResult(_pClassifierDelegate->getLastError());
    }

    _pClassifierDelegate->setNotifier(&_notifier);

    _pClassifierDelegate->setSeed((seed == 0) ? _seeds[SEED_CLASSIFIER] : seed);

    _bUseModel = !strModelFile.empty();
    _bPredictorLoaded = true;

    return tResult();
}


bool ClassificationTask::setupPredictor(const Mash::tExperimentParametersList& parameters)
{
    // Assertions
    assert(_pClassifierDelegate);

    return _pClassifierDelegate->setup(parameters);
}


Mash::FeaturesComputer* ClassificationTask::getFeaturesComputer()
{
    return _inputSet.featuresComputer();
}


ServerListener::tAction ClassificationTask::train()
{
    // Assertions
    assert(_pListener);
    assert(_pInstrumentsSet);
    assert(_pClassifierDelegate);

    // Create the instruments if necessary
    tError error = createInstruments();
    if (error != ERROR_NONE)
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(error)))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Initialize the set of heuristics
    if (!_inputSet.featuresComputer()->initialized())
    {
        if (!_inputSet.featuresComputer()->init(1, _inputSet.getDataSet()->roiExtent()))
        {
            if (!_pListener->sendResponse("ERROR", getErrorDescription(_inputSet.featuresComputer()->getLastError())))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }
    }

    // If necessary, load the predictor model
    if (_bUseModel)
    {
        if (!_pClassifierDelegate->loadModel(&_inputSet))
        {
            if (!_pListener->sendResponse("ERROR", ArgumentsList("Failed to load the predictor model")))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }
    }
    
    // Notify the instruments about the start of the experiment
    if (!_pInstrumentsSet->onExperimentStarted(&_inputSet))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Setup the Input Set
    _inputSet.getDataSet()->setMode(DataSet::MODE_TRAINING);
    _inputSet.onUpdated();

    // Notify the instruments about the start of the training
    if (!_pInstrumentsSet->onClassifierTrainingStarted(&_inputSet))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Train the classifier
    scalar_t train_error = 0 * log(0);
    if (!_pClassifierDelegate->train(&_inputSet, train_error))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pClassifierDelegate->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Retrieve the train error
    if (isnan(train_error) && !classify(&train_error, false))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pClassifierDelegate->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Notify the instruments
    if (!_pInstrumentsSet->onClassifierTrainingDone(&_inputSet, train_error))
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
    if (!_pListener->sendResponse("TRAIN_ERROR", train_error))
        return ServerListener::ACTION_CLOSE_CONNECTION;

    return ServerListener::ACTION_NONE;
}


Mash::ServerListener::tAction ClassificationTask::test()
{
    // Assertions
    assert(_pListener);
    assert(_pInstrumentsSet);
    assert(_pClassifierDelegate);

    // If no training was done (because a model must be loaded), do the missing
    // initialization steps
    if (!_bInstrumentsCreated)
    {
        assert(_bUseModel);
        
        // Create the instruments
        tError error = createInstruments();
        if (error != ERROR_NONE)
        {
            if (!_pListener->sendResponse("ERROR", getErrorDescription(error)))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }

        // Initialize the set of heuristics
        if (!_inputSet.featuresComputer()->initialized())
        {
            if (!_inputSet.featuresComputer()->init(1, _inputSet.getDataSet()->roiExtent()))
            {
                if (!_pListener->sendResponse("ERROR", getErrorDescription(_inputSet.featuresComputer()->getLastError())))
                    return ServerListener::ACTION_CLOSE_CONNECTION;

                return ServerListener::ACTION_NONE;
            }
        }

        // If necessary, load the predictor model
        if (!_pClassifierDelegate->loadModel(&_inputSet))
        {
            if (!_pListener->sendResponse("ERROR", ArgumentsList("Failed to load the predictor model")))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }
    
        // Notify the instruments about the start of the experiment
        if (!_pInstrumentsSet->onExperimentStarted(&_inputSet))
        {
            if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
                return ServerListener::ACTION_CLOSE_CONNECTION;

            return ServerListener::ACTION_NONE;
        }
    }

    // Setup the Input Set
    _inputSet.getDataSet()->setMode(DataSet::MODE_TEST);
    _inputSet.onUpdated();

    // Notify the instruments
    if (!_pInstrumentsSet->onClassifierTestStarted(&_inputSet))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Classification
    scalar_t test_error;
    if (!classify(&test_error, true))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pClassifierDelegate->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Notify the instruments
    if (!_pInstrumentsSet->onClassifierTestDone(&_inputSet, test_error))
    {
        if (!_pListener->sendResponse("ERROR", getErrorDescription(_pInstrumentsSet->getLastError())))
            return ServerListener::ACTION_CLOSE_CONNECTION;

        return ServerListener::ACTION_NONE;
    }

    // Sends a response to the client
    if (!_pListener->sendResponse("TEST_ERROR", test_error))
        return ServerListener::ACTION_CLOSE_CONNECTION;

    return ServerListener::ACTION_NONE;
}


bool ClassificationTask::getFeaturesUsed(Mash::tFeatureList &list)
{
    // Assertions
    assert(_pClassifierDelegate);

    // Ask the classifier
    return _pClassifierDelegate->reportFeaturesUsed(&_inputSet, list);
}


bool ClassificationTask::savePredictorModel()
{
    // Assertions
    assert(_pClassifierDelegate);

    // Ask the classifier
    return _pClassifierDelegate->saveModel();
}


Mash::ServerListener::tAction ClassificationTask::reportErrors()
{
    // Assertions
    assert(_pListener);

    // Retrieve the errors
    SandboxedClassifier* pSandboxedClassifier = dynamic_cast<SandboxedClassifier*>(_pClassifierDelegate);

    tError classifier_error = (_pClassifierDelegate ? _pClassifierDelegate->getLastError() : ERROR_NONE);
    tError heuristic_error  = (_inputSet.featuresComputer() ? _inputSet.featuresComputer()->getLastError() : ERROR_NONE);
    tError instrument_error = (_pInstrumentsSet ? _pInstrumentsSet->getLastError() : ERROR_NONE);

    if (classifier_error == ERROR_CHANNEL_SLAVE_CRASHED)
        classifier_error = ERROR_CLASSIFIER_CRASHED;

    if (heuristic_error == Mash::ERROR_CHANNEL_SLAVE_CRASHED)
        heuristic_error = Mash::ERROR_HEURISTIC_CRASHED;

    if (instrument_error == ERROR_CHANNEL_SLAVE_CRASHED)
        instrument_error = ERROR_INSTRUMENT_CRASHED;


    // Test if the classifier crashed
    if (classifier_error == ERROR_CLASSIFIER_CRASHED)
    {
        ServerListener::tAction action = sendErrorReport(
                             "PREDICTOR_CRASH", ArgumentsList(),
                             (pSandboxedClassifier ? pSandboxedClassifier->getContext() : ""),
                             true, (pSandboxedClassifier ? pSandboxedClassifier->getStackTrace() : ""));

        if (action != ServerListener::ACTION_NONE)
            return action;
    }

    // Test if the classifier used a forbidden system call
    else if (classifier_error == ERROR_SANDBOX_FORBIDDEN_SYSTEM_CALL)
    {
        assert(pSandboxedClassifier);

        ServerListener::tAction action = sendErrorReport(
                             "PREDICTOR_ERROR",
                             getErrorDescription(classifier_error) + ": " + pSandboxedClassifier->sandboxController()->getLastErrorDetails(),
                             pSandboxedClassifier->getContext());

        if (action != ServerListener::ACTION_NONE)
            return action;
    }

    // Test if the warden failed
    else if (classifier_error == ERROR_WARDEN)
    {
        assert(pSandboxedClassifier);

        ServerListener::tAction action = sendErrorReport(
                             "PREDICTOR_ERROR",
                             pSandboxedClassifier->sandboxController()->getLastErrorDetails(),
                             pSandboxedClassifier->getContext());

        if (action != ServerListener::ACTION_NONE)
            return action;
    }

    // Other classifier errors
    else if ((classifier_error != ERROR_NONE) && (classifier_error != ERROR_HEURISTIC_TIMEOUT) &&
             (classifier_error != ERROR_HEURISTIC_CRASHED))
    {
        ServerListener::tAction action = sendErrorReport(
                             "PREDICTOR_ERROR",
                             getErrorDescription(classifier_error),
                             (pSandboxedClassifier ? pSandboxedClassifier->getContext() : ""));

        if (action != ServerListener::ACTION_NONE)
            return action;
    }


    // Process the error of the heuristics (if any)
    if (heuristic_error != ERROR_NONE)
    {
        ServerListener::tAction action = sendHeuristicsErrorReport(_inputSet.featuresComputer(),
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
    if ((classifier_error == ERROR_NONE) && (heuristic_error == ERROR_NONE) && (instrument_error == ERROR_NONE))
    {
        if (!_pListener->sendResponse("NO_ERROR", ArgumentsList()))
            return ServerListener::ACTION_CLOSE_CONNECTION;
    }

    return ServerListener::ACTION_NONE;
}


unsigned int ClassificationTask::getNbLogFiles()
{
    // Assertions
    assert(_pClassifierDelegate);

    SandboxedHeuristicsSet* pSandboxedHeuristicsSet = dynamic_cast<SandboxedHeuristicsSet*>(getFeaturesComputer()->heuristicsSet());
    SandboxedClassifier*    pSandboxedClassifier    = dynamic_cast<SandboxedClassifier*>(_pClassifierDelegate);

    return TaskController::getNbLogFiles() +
           (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->sandboxController()->getNbLogFiles() : 0) +
           (pSandboxedClassifier ? pSandboxedClassifier->sandboxController()->getNbLogFiles() : 0);
}


int ClassificationTask::getLogFileContent(unsigned int index, std::string& strName,
                                          unsigned char** pBuffer, int64_t max_size)
{
    // Assertions
    assert(_pClassifierDelegate);

    unsigned int nb = TaskController::getNbLogFiles();
    if (index < nb)
        return TaskController::getLogFileContent(index, strName, pBuffer, max_size);

    index -= nb;


    SandboxedHeuristicsSet* pSandboxedHeuristicsSet = dynamic_cast<SandboxedHeuristicsSet*>(getFeaturesComputer()->heuristicsSet());
    nb = (pSandboxedHeuristicsSet ? pSandboxedHeuristicsSet->sandboxController()->getNbLogFiles() : 0);
    if (index < nb)
        return pSandboxedHeuristicsSet->sandboxController()->getLogFileContent(index, strName, pBuffer, max_size);

    index -= nb;


    SandboxedClassifier* pSandboxedClassifier = dynamic_cast<SandboxedClassifier*>(_pClassifierDelegate);
    nb = (pSandboxedClassifier ? pSandboxedClassifier->sandboxController()->getNbLogFiles() : 0);
    if (index < nb)
        return pSandboxedClassifier->sandboxController()->getLogFileContent(index, strName, pBuffer, max_size);


    *pBuffer = 0;
    strName = "";
    return 0;
}


void ClassificationTask::fillReport(const std::string& strReportFolder)
{
    // Report the list of labels
    TiXmlDocument xmlDoc;

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
    xmlDoc.LinkEndChild(decl);

    TiXmlElement* xmlLabels = new TiXmlElement("labels");
    xmlDoc.LinkEndChild(xmlLabels);

    unsigned int nbLabels = _inputSet.nbLabels();
    for (unsigned int i = 0; i < nbLabels; ++i)
    {
        string strName = _inputSet.getDatabase()->labelName(i);

        TiXmlElement* xmlLabel = new TiXmlElement("label");
        xmlLabel->SetAttribute("index", i);
        xmlLabel->SetAttribute("name", strName);

        xmlLabels->LinkEndChild(xmlLabel);
    }

    xmlDoc.SaveFile(strReportFolder + "labels.xml");


    // Report the list of images
    xmlDoc = TiXmlDocument();

    decl = new TiXmlDeclaration("1.0", "", "");
    xmlDoc.LinkEndChild(decl);

    TiXmlElement* xmlImages = new TiXmlElement("images");
    xmlDoc.LinkEndChild(xmlImages);

    unsigned int nbImages = _inputSet.getDataSet()->nbImages();
    for (unsigned int i = 0; i < nbImages; ++i)
    {
        string          strName;
        dim_t           size;
        scalar_t        scale;
        bool            training;
        unsigned int    set_index;

        _inputSet.getDataSet()->getImageInfos(i, &strName, &size, &scale,
                                              &training, &set_index);

        TiXmlElement* xmlImage = new TiXmlElement("image");
        xmlImage->SetAttribute("index", i);
        xmlImage->SetAttribute("name", strName);
        xmlImage->SetAttribute("width", size.width);
        xmlImage->SetAttribute("height", size.height);
        xmlImage->SetDoubleAttribute("scale", scale);

        if (training)
            xmlImage->SetAttribute("training_index", set_index);
        else
            xmlImage->SetAttribute("test_index", set_index);

        xmlImages->LinkEndChild(xmlImage);
    }

    xmlDoc.SaveFile(strReportFolder + "images.xml");
}


/********************************** METHODS ***********************************/

struct tDetection : public tObject
{
    scalar_t score;     // Score is equal to max(results)
    unsigned int image; // Image in which the detection was made

    // There is no need for results since an instrument would be incapable of finding back the correct object
//  Classifier::tClassificationResults results;

    bool operator<(const tDetection& d) const
    {
        return score < d.score;
    }
};


bool ClassificationTask::classify(scalar_t* result, bool bDoingTest)
{
    // Assertions
    assert(isPredictorLoaded());
    assert(_inputSet.getStepper());
    assert(_pClassifierDelegate);
    assert(_pInstrumentsSet);
    assert(result);

    // Constants used both for classification and detection
    const Stepper* stepper = _inputSet.getStepper();
    const unsigned int roi_extent = stepper->roiExtent();
    const unsigned int roi_size = roi_extent * 2 + 1;

    // Classification
    if (!stepper->isDoingDetection())
    {
        // The number of missclassifications
        unsigned int nbErrors = 0;

        if (bDoingTest)
            _notifier.onTestStepDone(0, _inputSet.nbImages());
        else
            _notifier.onTrainErrorComputationStepDone(0, _inputSet.nbImages());

        if (_inputSet.nbImages() >= 1000)
            _notifier.setReductor(100);
        else if (_inputSet.nbImages() >= 100)
            _notifier.setReductor(10);

        for (unsigned int image = 0; image < _inputSet.nbImages(); ++image)
        {
            // Get the dimensions of the image
            dim_t image_size = _inputSet.imageSize(image);

            // The stepper should return only one position (the center)
            tCoordinatesList positions;
            stepper->getPositions(image_size, &positions);
            assert(positions.size() == 1);

            _inputSet.restrictAccess(true);

            // Obtain the classifier response at that position
            Classifier::tClassificationResults results;
            bool ret = _pClassifierDelegate->classify(&_inputSet, image, positions.front(), results);

            _inputSet.restrictAccess(false);

            if (!ret)
                return false;

            // Assume that the classifier made an error for the moment
            ++nbErrors;

            Instrument::tClassificationError classificationError = Instrument::CLASSIFICATION_ERROR_FALSE_REJECTION;

            // It is an error to not return any result
            if (!results.empty())
            {
                classificationError = Instrument::CLASSIFICATION_ERROR_WRONG_CLASSIFICATION;

                // Find the label with the biggest score
                Classifier::tClassificationResults::iterator iter =
                    max_element(results.begin(), results.end(), value_comparer);

                // Get the objects of the image
                tObjectsList objects;
                _inputSet.objectsInImage(image, &objects);

                // Cancel the assumed error if there is an object with that label
                tObjectsList::iterator iter2, iterEnd2;
                for (iter2 = objects.begin(), iterEnd2 = objects.end(); iter2 != iterEnd2; ++iter2)
                {
                    if (iter2->label == iter->first)
                    {
                        classificationError = Instrument::CLASSIFICATION_ERROR_NONE;
                        --nbErrors;
                        break;
                    }
                }
            }

            _pInstrumentsSet->onClassifierClassificationDone(&_inputSet, image,
                                                             _inputSet.getDataSet()->getImageIndex(image),
                                                             positions.front(), results, classificationError);

            if (bDoingTest)
                _notifier.onTestStepDone(image + 1);
            else
                _notifier.onTrainErrorComputationStepDone(image + 1);
        }

        _notifier.setReductor(0);

        // Returns the percentage of errors
        *result = (scalar_t) nbErrors / _inputSet.nbImages();
        return true;
    }

    // Detection
    else
    {
        // Cluster together detections of the same image accross scale
        // Assume scales of a same image are consecutive

        // Identify images by their names
        string previous_name;

        // The first index of the current image
        unsigned int first_image = 0;

        // Detections of the current image (accross scales)
        vector<tDetection> detections;

        // Number of correctly detected and incorrectly or non detected objects
        unsigned int nb_detected = 0;
        unsigned int nb_non_detected = 0;

        for (unsigned int image = 0; image <= _inputSet.nbImages(); ++image)
        {
            // Get the info of the current image
            string image_name;
            dim_t image_size;
            scalar_t image_scale;
            bool image_training;
            unsigned int image_set_index;

            if (image < _inputSet.nbImages())
            {
                _inputSet.getDataSet()->getImageInfos(_inputSet.getDataSet()->getImageIndex(image), &image_name,
                                                      &image_size, &image_scale, &image_training, &image_set_index);
            }

            if (image_training)
                break;

            // If the image is really a new one, report the detections of the previous one
            if (image_name != previous_name || image == _inputSet.nbImages())
            {
                // Get all the objects at all the scales
                vector<tObjectsList> objects(image - first_image);

                for (unsigned int i = first_image; i < image; ++i)
                    _inputSet.objectsInImage(i, &objects[i - first_image]);

                // Flag each detected object
                vector<bool> detected(objects.empty() ? 0 : objects[0].size(), false);

                // Flag each intersected object
                vector<bool> intersected(detected.size(), false);

                // For each detection, determine if it correct, wrong, or a false alarm
                vector<tDetection>::const_iterator iter2, iterEnd2;
                for (iter2 = detections.begin(), iterEnd2 = detections.end(); iter2 != iterEnd2; ++iter2)
                {
                    // Assume for the moment that this detection is a false alarm
                    Instrument::tClassificationError detectionError = Instrument::CLASSIFICATION_ERROR_FALSE_ALARM;

                    // Look at all the objects that this detection intersect with
                    tObjectIntersecter intersecter(*iter2, 0.5f);

                    // Check each object of the detection image for intersection
                    const tObjectsIterator iterBegin3 = objects[iter2->image - first_image].begin();
                    const tObjectsIterator iterEnd3   = objects[iter2->image - first_image].end();
                    for (tObjectsIterator iter3 = iterBegin3; iter3 != iterEnd3; ++iter3)
                    {
                        if (intersecter(*iter3))
                        {
                            if (iter2->label == iter3->label)
                            {
                                // Flag the detection as correct
                                detectionError = Instrument::CLASSIFICATION_ERROR_NONE;

                                // Report the detection if the object has not been already reported
                                // It is very important for correct detection to be reported immediately as a correct detection
                                // might detect multiple objects, and thus might have to report to instruments multiple times
                                if (!detected[iter3 - iterBegin3])
                                {
                                    Classifier::tClassificationResults results;
                                    results[iter2->label] = iter2->score;
                                    _pInstrumentsSet->onClassifierClassificationDone(&_inputSet, iter2->image,
                                                                                     _inputSet.getDataSet()->getImageIndex(iter2->image),
                                                                                     iter2->roi_position, results, detectionError);
                                    detected[iter3 - iterBegin3] = true;
                                    ++nb_detected;
                                }
                            }
                            // Must be careful not to modify a correct detection into a wrong one
                            else if (detectionError != Instrument::CLASSIFICATION_ERROR_NONE)
                            {
                                detectionError = Instrument::CLASSIFICATION_ERROR_WRONG_CLASSIFICATION;
                                ++nb_non_detected;
                            }

                            intersected[iter3 - iterBegin3] = true;
                        }
                    }

                    // Report a wrong detection or a false alarm
                    if (detectionError == Instrument::CLASSIFICATION_ERROR_WRONG_CLASSIFICATION ||
                        detectionError == Instrument::CLASSIFICATION_ERROR_FALSE_ALARM)
                    {
                        Classifier::tClassificationResults results;
                        results[iter2->label] = iter2->score;
                        _pInstrumentsSet->onClassifierClassificationDone(&_inputSet, iter2->image,
                                                                         _inputSet.getDataSet()->getImageIndex(iter2->image),
                                                                         iter2->roi_position, results, detectionError);
                    }
                }

                // Report a CLASSIFICATION_ERROR_FALSE_REJECTION error for each non-intersected object
                for (unsigned int i = 0; i < intersected.size(); ++i)
                {
                    if (!intersected[i])
                    {
                        // Report the object as in the image in which it is a target
                        for (unsigned int j = 0; j < objects.size(); ++j)
                        {
                            if (objects[j][i].target)
                            {
                                _pInstrumentsSet->onClassifierClassificationDone(&_inputSet, first_image + j,
                                                                                 _inputSet.getDataSet()->getImageIndex(first_image + j),
                                                                                 objects[j][i].roi_position, Classifier::tClassificationResults(),
                                                                                 Instrument::CLASSIFICATION_ERROR_FALSE_REJECTION);
                                ++nb_non_detected;
                                break;
                            }
                        }
                    }
                }

                first_image = image;
                previous_name = image_name;
                vector<tDetection>().swap(detections);
            }

            // Break if there is no image after this one
            if (image == _inputSet.nbImages())
                break;

            // Get the dimensions of the image (with scaling applied)
            image_size = _inputSet.imageSize(image);

            // Iterate over all the positions returned by the stepper
            tCoordinatesList positions;
            stepper->getPositions(image_size, &positions);

            // There should always be at least one position (the center)
            assert(positions.size() > 0);

            // Scanning (at each position find the label with the biggest score)
            // TODO: do clustering here rather than later to improve efficiency
            tCoordinatesIterator iter, iterEnd;
            for (iter = positions.begin(), iterEnd = positions.end(); iter != iterEnd; ++iter)
            {
                _inputSet.restrictAccess(true);

                // Obtain the classifier response at that position
                Classifier::tClassificationResults results;
                bool ret = _pClassifierDelegate->classify(&_inputSet, image, *iter, results);

                _inputSet.restrictAccess(false);

                if (!ret)
                    return false;

                // No need to consider the empty case (no result => no detection => zero detection rate)
                if (!results.empty())
                {
                    // Find the label with the biggest score
                    Classifier::tClassificationResults::const_iterator iterResult =
                        max_element(results.begin(), results.end(), value_comparer);

                    // Add a detection at that position
                    tDetection detection;
                    detection.label = iterResult->first;
                    detection.target = false;
                    detection.roi_position = *iter;
                    detection.roi_extent = roi_extent;
                    detection.score = iterResult->second;
                    detection.image = image;
                //  detection.results.swap(results);

                    // Look if there is already a detection with a higher score intersecting with this one
                    tObjectIntersecter intersecter(detection, 0.25f);

                    vector<tDetection>::const_iterator iter2, iterEnd2;
                    for (iter2 = detections.begin(), iterEnd2 = detections.end(); iter2 != iterEnd2; ++iter2)
                        if (iter2->score >= detection.score && intersecter(*iter2))
                            break;

                    // Remove all the previous detection intersecting with this one before adding it
                    if (iter2 == iterEnd2)
                    {
                        iter2 = remove_if(detections.begin(), detections.end(), intersecter);
                        detections.resize(iter2 - detections.begin());
                        detections.push_back(detection);
                    }
                }
            }
        }

        // Returns the percentage of incorrectly or non detected objects
        *result = (scalar_t) nb_non_detected / (nb_detected + nb_non_detected);
        return true;
    }
}
