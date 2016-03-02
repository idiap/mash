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


/** @file   classification_task.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ClassificationTask' class
*/

#ifndef _CLASSIFICATION_TASK_H_
#define _CLASSIFICATION_TASK_H_

#include "task_controller.h"
#include <mash-classification/classifier_input_set.h>
#include <mash-classification/classifier_delegate.h>
#include <mash-instrumentation/classifier_input_set_listener.h>


//------------------------------------------------------------------------------
/// @brief  Task Controller for Classification and Object Detection
//------------------------------------------------------------------------------
class ClassificationTask: public TaskController
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    //--------------------------------------------------------------------------
    ClassificationTask(Listener* pListener, bool detection);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~ClassificationTask();


    //_____ Methods __________
public:
    //----------------------------------------------------------------------
    /// @brief  Indicates if the task is an object detection one
    //----------------------------------------------------------------------
    inline bool isDoingDetection() const
    {
        return _inputSet.isDoingDetection();
    }

protected:
    //----------------------------------------------------------------------
    /// @brief  Perform a classification/detection on all the images in the
    ///         Input Set
    //----------------------------------------------------------------------
    bool classify(Mash::scalar_t* result, bool bDoingTest);


    //_____ Implementation of TaskController __________
public:
    //----------------------------------------------------------------------
    /// @copy TaskController::setup
    //----------------------------------------------------------------------
    virtual Mash::tError setup(const tTaskControllerConfiguration& configuration);

    //----------------------------------------------------------------------
    /// @copy TaskController::setGlobalSeed
    //----------------------------------------------------------------------
    virtual void setGlobalSeed(unsigned int seed);

    //----------------------------------------------------------------------
    /// @copy TaskController::setClient
    //----------------------------------------------------------------------
    virtual Mash::tError setClient(Mash::Client* pClient);

    //----------------------------------------------------------------------
    /// @copy TaskController::getClient
    //----------------------------------------------------------------------
    virtual Mash::Client* getClient();

    //--------------------------------------------------------------------------
    /// @copy TaskController::getFeaturesComputer
    //--------------------------------------------------------------------------
    virtual Mash::FeaturesComputer* getFeaturesComputer();

    //----------------------------------------------------------------------
    /// @copy TaskController::setParameters
    //----------------------------------------------------------------------
    virtual tResult setParameters(const Mash::tExperimentParametersList& parameters);

    //----------------------------------------------------------------------
    /// @copy TaskController::loadPredictor
    //----------------------------------------------------------------------
    virtual tResult loadPredictor(const std::string strName,
                                  const std::string& strModelFile = "",
                                  const std::string& strInternalDataFile = "",
                                  unsigned int seed = 0);

    //----------------------------------------------------------------------
    /// @copy TaskController::setupPredictor
    //----------------------------------------------------------------------
    virtual bool setupPredictor(const Mash::tExperimentParametersList& parameters);

    //----------------------------------------------------------------------
    /// @copy TaskController::train
    //----------------------------------------------------------------------
    virtual Mash::ServerListener::tAction train();

    //----------------------------------------------------------------------
    /// @copy TaskController::test
    //----------------------------------------------------------------------
    virtual Mash::ServerListener::tAction test();

    //----------------------------------------------------------------------
    /// @copy TaskController::getFeaturesUsed
    //----------------------------------------------------------------------
    virtual bool getFeaturesUsed(Mash::tFeatureList &list);

    //--------------------------------------------------------------------------
    /// @copy TaskController::savePredictorModel
    //--------------------------------------------------------------------------
    virtual bool savePredictorModel();

    //----------------------------------------------------------------------
    /// @copy TaskController::reportErrors
    //----------------------------------------------------------------------
    virtual Mash::ServerListener::tAction reportErrors();

    //--------------------------------------------------------------------------
    /// @copy TaskController::getNbLogFiles
    //--------------------------------------------------------------------------
    virtual unsigned int getNbLogFiles();

    //--------------------------------------------------------------------------
    /// @copy TaskController::getLogFileContent
    //--------------------------------------------------------------------------
    virtual int getLogFileContent(unsigned int index, std::string& strName,
                                  unsigned char** pBuffer, int64_t max_size = 0);

    //--------------------------------------------------------------------------
    /// @copy TaskController::fillReport
    //--------------------------------------------------------------------------
    virtual void fillReport(const std::string& strReportFolder);


    //_____ Internal types __________
public:
    enum tSeed
    {
        SEED_IMAGES,
        SEED_HEURISTICS,
        SEED_CLASSIFIER,
        SEED_APPSERVER,
        
        COUNT_SEEDS
    };
    

    //_____ Attributes __________
protected:
    Mash::IClassifierDelegate*          _pClassifierDelegate;
    Mash::ClassifierInputSet            _inputSet;
    Mash::ClassifierInputSetListener    _listener;
    unsigned int                        _seeds[COUNT_SEEDS];
    bool                                _bUseModel;
};

#endif
