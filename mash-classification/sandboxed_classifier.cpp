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
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-classification/classifier_input_set.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <sstream>


using namespace std;
using namespace Mash;
using namespace Mash::SandboxControllerDeclarations;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxedClassifier::SandboxedClassifier()
: _pInputSetProxy(0), _pNotifierProxy(0)
{
}


SandboxedClassifier::~SandboxedClassifier()
{
    _outStream.deleteFile();
    
    delete _pNotifierProxy;
}


/***************************** SANDBOX MANAGEMENT *****************************/

bool SandboxedClassifier::createSandbox(const tSandboxConfiguration& configuration)
{
    _outStream.setVerbosityLevel(3);
    _outStream.open("PredictorSandboxController",
                    configuration.strLogDir + "PredictorSandboxController_$TIMESTAMP.log",
                    200 * 1024);

    _sandbox.setOutputStream(_outStream);
    _sandbox.addLogFileInfos("PredictorSandbox");

    _sandbox.addLogFileInfos("Predictor", true, true);

    if (!_sandbox.createSandbox(PLUGIN_CLASSIFIER, configuration, this))
        return false;
    
    _pNotifierProxy = new SandboxNotifierProxy(0, *_sandbox.channel());
    return true;
}


/**************************** CLASSIFIER MANAGEMENT ***************************/

bool SandboxedClassifier::setClassifiersFolder(const std::string& strPath)
{
    return _sandbox.setPluginsFolder(strPath);
}


bool SandboxedClassifier::loadClassifierPlugin(const std::string& strName,
                                               const std::string& strModelFile,
                                               const std::string& strInternalDataFile)
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

        if (!strInternalDataFile.empty())
        {
            str << "    - Internal data" << endl;
            _outStream << "< USE_MODEL " << strModelFile << " " << strInternalDataFile << endl;
        }
        else
        {
            str << "    - No internal data" << endl;
            _outStream << "< USE_MODEL " << strModelFile << " -" << endl;
        }
    }
    else
    {
        str << "    - No model" << endl
            << "    - No internal data" << endl;
        _outStream << "< USE_MODEL -" << endl;
    }

    _strContext = str.str();

    // Send the infos about the model to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_USE_MODEL);
    pChannel->add(!strModelFile.empty() ? strModelFile : "-");
    pChannel->add((!strModelFile.empty() && !strInternalDataFile.empty()) ? strInternalDataFile : "-");
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


void SandboxedClassifier::setNotifier(INotifier* pNotifier)
{
    delete _pNotifierProxy;
    _pNotifierProxy = new SandboxNotifierProxy(pNotifier, *_sandbox.channel());
}


/*********************************** METHODS **********************************/

bool SandboxedClassifier::setSeed(unsigned int seed)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _strContext = "";

    _outStream << "< SET_SEED " << seed << endl;

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_CLASSIFIER_SET_SEED);
    pChannel->add(seed);
    pChannel->sendPacket();

    if (!pChannel->good())
        return false;

    // Read the response
    return _sandbox.waitResponse();
}


bool SandboxedClassifier::setup(const tExperimentParametersList& parameters)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< SETUP " << parameters.size() << " ..." << endl;

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

    pChannel->startPacket(SANDBOX_COMMAND_CLASSIFIER_SETUP);
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

    return result;
}


bool SandboxedClassifier::loadModel(IClassifierInputSet* input_set)
{
    // Assertions
    assert(input_set);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< LOAD_MODEL" << endl;

    // Save the context (in case of crash)
    std::ostringstream str;
    str << "Method: loadModel" << endl;
    _strContext = str.str();

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Send the command to the child
    pChannel->startPacket(SANDBOX_COMMAND_LOAD_MODEL);
    pChannel->sendPacket();

    // Read the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();

    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    return result && pChannel->good();
}


bool SandboxedClassifier::train(IClassifierInputSet* input_set, scalar_t &train_error)
{
    // Assertions
    assert(input_set);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< TRAIN " << input_set->id() << " " << input_set->isDoingDetection() << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

    str << "Method: train" << endl
        << "Parameters:" << endl
        << "    - Number of images:     " << input_set->nbImages() << endl
        << "    - Number of labels:     " << input_set->nbLabels() << endl
        << "    - Number of heuristics: " << input_set->nbHeuristics() << endl
        << "    - Number of features:   " << input_set->nbFeaturesTotal() << endl
        << "    - ROI extent:           " << input_set->roiExtent() << " pixels" << endl;

    _strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_COMMAND_CLASSIFIER_TRAIN);
    pChannel->add(input_set->id());
    pChannel->add(input_set->isDoingDetection());
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();
    
    if (result && !pChannel->endOfPacket())
    {
        // Retrieve the train error
        pChannel->read(&train_error);
    }
    
    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    return result;
}


bool SandboxedClassifier::classify(IClassifierInputSet* input_set,
                                   unsigned int image,
                                   const coordinates_t& position,
                                   Classifier::tClassificationResults &results)
{
    // Assertions
    assert(input_set);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< CLASSIFY " << " " << input_set->id() << " "
               << input_set->isDoingDetection() << " " << image << " "
               << position.x << " " << position.y << endl;

    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    // Save the context (in case of crash)
    std::ostringstream str;

    dim_t image_size = input_set->imageSize(image);

    str << "Method: classify" << endl
        << "Parameters:" << endl
        << "    - Image:                #" << image << " (" << image_size.width << "x" << image_size.height << " pixels)" << endl
        << "    - ROI extent:           " << input_set->roiExtent() << " pixels" << endl
        << "    - ROI position:         (" << position.x << ", " << position.y << ")" << endl
        << "    - Number of labels:     " << input_set->nbLabels() << endl
        << "    - Number of heuristics: " << input_set->nbHeuristics() << endl
        << "    - Number of features:   " << input_set->nbFeaturesTotal() << endl;

    _strContext = str.str();

    // Send the command to the child
    pChannel->startPacket(SANDBOX_COMMAND_CLASSIFIER_CLASSIFY);
    pChannel->add(input_set->id());
    pChannel->add(input_set->isDoingDetection());
    pChannel->add(image);
    pChannel->add(position.x);
    pChannel->add(position.y);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse();

    // Retrieve the results
    if (result)
    {
        unsigned int nbScores = 0;
        pChannel->read(&nbScores);

        for (unsigned int i = 0; pChannel->good() && (i < nbScores); ++i)
        {
            int label;
            scalar_t score;

            if (pChannel->read(&label) && pChannel->read(&score))
                results[label] = score;
        }
    }

    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    return result && pChannel->good();
}


bool SandboxedClassifier::reportFeaturesUsed(IClassifierInputSet* input_set,
                                             tFeatureList &list)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< REPORT_FEATURES_USED" << endl;

    // Save the context (in case of crash)
    std::ostringstream str;

    str << "Method: reportFeaturesUsed" << endl;

    _strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    _pInputSetProxy = new SandboxInputSetProxy(input_set, *pChannel);

    pChannel->startPacket(SANDBOX_COMMAND_CLASSIFIER_REPORT_FEATURES_USED);
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

    delete _pInputSetProxy;
    _pInputSetProxy = 0;

    return result && pChannel->good();
}


bool SandboxedClassifier::saveModel()
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


tError SandboxedClassifier::getLastError()
{
    return (_sandbox.getLastError() == ERROR_CHANNEL_SLAVE_CRASHED ?
                    ERROR_CLASSIFIER_CRASHED : _sandbox.getLastError());
}


tCommandProcessingResult SandboxedClassifier::processResponse(tSandboxMessage message)
{
    tCommandProcessingResult result = COMMAND_UNKNOWN;
    
    if (_pInputSetProxy)
        result = _pInputSetProxy->processResponse(message);

    if ((result == COMMAND_UNKNOWN) && _pNotifierProxy)
        result = _pNotifierProxy->processResponse(message);

    return result;
}
