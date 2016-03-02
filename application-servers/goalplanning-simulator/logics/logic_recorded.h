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


/** @file   logic_recorded.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'LogicRecorded' class
*/

#ifndef _LOGIC_RECORDED_H_
#define _LOGIC_RECORDED_H_

#include <logic_interface.h>


//------------------------------------------------------------------------------
/// @brief Logic able to read data (images, actions, rewards) from a recorded
///        database
//------------------------------------------------------------------------------
class LogicRecorded: public ILogic
{
    //_____ Internal types __________
public:
    struct tStep
    {
        Mash::tStringList images;
        std::string       action;
        float             reward;
    };

    typedef std::vector<tStep>          tStepList;
    typedef tStepList::iterator         tStepIterator;

    struct tTrajectory
    {
        tStepList steps;
        bool      finished;
        bool      failed;
    };

    typedef std::vector<tTrajectory>    tTrajectoryList;
    typedef tTrajectoryList::iterator   tTrajectoryItrator;


    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief Constructor
    //--------------------------------------------------------------------------
    LogicRecorded(const std::string& strDatabasePath, bool bLoadAll=true);

    //--------------------------------------------------------------------------
    /// @brief Destructor
    //--------------------------------------------------------------------------
    virtual ~LogicRecorded();


    //_____ Implementation of ILogic __________
public:
    //--------------------------------------------------------------------------
    /// @brief Returns the mode of the task
    //--------------------------------------------------------------------------
    virtual Mash::tGoalPlanningMode getMode()
    {
        return _mode;
    }

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the possible actions
    //--------------------------------------------------------------------------
    virtual Mash::tStringList getActions()
    {
        return _actions;
    }

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the available views
    //--------------------------------------------------------------------------
    virtual Mash::IApplicationServer::tViewsList getViews()
    {
        return _views;
    }

    //--------------------------------------------------------------------------
    /// @brief Returns the number of trajectories available
    ///
    /// Only valid in GPMODE_RECORDED_TEACHER and GPMODE_RECORDED_TRAJECTORIES
    /// modes
    //--------------------------------------------------------------------------
    virtual unsigned int getNbTrajectories()
    {
        return _trajectories.size();
    }

    //--------------------------------------------------------------------------
    /// @brief Returns the number of actions in a trajectory
    ///
    /// @param trajectory   Index of the trajectory
    ///
    /// Only valid in GPMODE_RECORDED_TEACHER and GPMODE_RECORDED_TRAJECTORIES
    /// modes
    //--------------------------------------------------------------------------
    virtual unsigned int getTrajectoryLength(unsigned int trajectory)
    {
        return _trajectories[trajectory].steps.size() - 1;
    }

    //--------------------------------------------------------------------------
    /// @brief Reset the task in its initial state
    //--------------------------------------------------------------------------
    virtual void reset(Mash::RandomNumberGenerator* generator);
    
    //--------------------------------------------------------------------------
    /// @brief Returns the filename containing the current image for one of the
    ///         views
    ///
    /// @param  viewName    The name of the view
    /// @return             The filename
    //--------------------------------------------------------------------------
    virtual const std::string getViewFileName(const std::string& viewName);
    
    //--------------------------------------------------------------------------
    /// @brief Performs an action
    ///
    /// @param      action          The name of the action to perform
    /// @param[out] reward          The reward
    /// @param[out] finished        'true' if the goal was reached
    /// @param[out] failed          'true' if the task was failed
    //--------------------------------------------------------------------------
    virtual void performAction(const std::string& action, float &reward,
                               bool &finished, bool &failed);

    //--------------------------------------------------------------------------
    /// @brief Returns the suggested action (if the mode requires it)
    //--------------------------------------------------------------------------
    virtual std::string getSuggestedAction()
    {
        return _trajectories[_currentTrajectory].steps[_currentStep].action;
    }


    //_____ Attributes __________
protected:
    Mash::tGoalPlanningMode                 _mode;
    Mash::tStringList                       _actions;
    Mash::IApplicationServer::tViewsList    _views;
    tTrajectoryList                         _trajectories;
    unsigned int                            _currentTrajectory;
    unsigned int                            _currentStep;
    bool                                    _firstResetDone;
};

#endif
