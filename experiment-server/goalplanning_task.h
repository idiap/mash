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


/** @file   goalplanning_task.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'GoalPlanningTask' class
*/

#ifndef _GOALPLANNING_TASK_H_
#define _GOALPLANNING_TASK_H_

#include "task_controller.h"
#include <mash-goalplanning/task.h>
#include <mash-goalplanning/planner_delegate.h>
#include <mash-instrumentation/perception_listener.h>


//------------------------------------------------------------------------------
/// @brief  Task Controller for Goal-planning
//------------------------------------------------------------------------------
class GoalPlanningTask: public TaskController
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    //--------------------------------------------------------------------------
    GoalPlanningTask(Listener* pListener);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~GoalPlanningTask();


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
        SEED_HEURISTICS,
        SEED_PLANNER,
        SEED_APPSERVER,

        COUNT_SEEDS
    };


    //_____ Attributes __________
public:
    Mash::IPlannerDelegate*     _pPlannerDelegate;
    Mash::Task                  _task;
    Mash::PerceptionListener    _listener;
    unsigned int                _seeds[COUNT_SEEDS];
    bool                        _bUseModel;
};

#endif
