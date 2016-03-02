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


/** @file   logic_interface.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ILogic' interface
*/

#ifndef _ILOGIC_H_
#define _ILOGIC_H_

#include <mash-appserver/application_server_interface.h>
#include <mash-utils/random_number_generator.h>


//------------------------------------------------------------------------------
/// @brief Interface of an object defining the logic of a task
//------------------------------------------------------------------------------
class ILogic
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief Constructor
    //--------------------------------------------------------------------------
    ILogic()
    {
    }

    //--------------------------------------------------------------------------
    /// @brief Destructor
    //--------------------------------------------------------------------------
    virtual ~ILogic()
    {
    }


    //_____ Methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief Returns the mode of the task
    //--------------------------------------------------------------------------
    virtual Mash::tGoalPlanningMode getMode() = 0;

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the possible actions
    //--------------------------------------------------------------------------
    virtual Mash::tStringList getActions() = 0;

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the available views
    //--------------------------------------------------------------------------
    virtual Mash::IApplicationServer::tViewsList getViews() = 0;

    //--------------------------------------------------------------------------
    /// @brief Returns the number of trajectories available
    ///
    /// Only valid in GPMODE_RECORDED_TEACHER and GPMODE_RECORDED_TRAJECTORIES
    /// modes
    //--------------------------------------------------------------------------
    virtual unsigned int getNbTrajectories() = 0;

    //--------------------------------------------------------------------------
    /// @brief Returns the number of actions in a trajectory
    ///
    /// @param trajectory   Index of the trajectory
    ///
    /// Only valid in GPMODE_RECORDED_TEACHER and GPMODE_RECORDED_TRAJECTORIES
    /// modes
    //--------------------------------------------------------------------------
    virtual unsigned int getTrajectoryLength(unsigned int trajectory) = 0;

    //--------------------------------------------------------------------------
    /// @brief Reset the task in its initial state
    //--------------------------------------------------------------------------
    virtual void reset(Mash::RandomNumberGenerator* generator) = 0;
    
    //--------------------------------------------------------------------------
    /// @brief Returns the filename containing the current image for one of the
    ///         views
    ///
    /// @param  viewName    The name of the view
    /// @return             The filename
    //--------------------------------------------------------------------------
    virtual const std::string getViewFileName(const std::string& viewName) = 0;
    
    //--------------------------------------------------------------------------
    /// @brief Performs an action
    ///
    /// @param      action          The name of the action to perform
    /// @param[out] reward          The reward
    /// @param[out] finished        'true' if the goal was reached
    /// @param[out] failed          'true' if the task was failed
    //--------------------------------------------------------------------------
    virtual void performAction(const std::string& action, float &reward,
                               bool &finished, bool &failed) = 0;

    //--------------------------------------------------------------------------
    /// @brief Returns the suggested action (if the mode requires it)
    //--------------------------------------------------------------------------
    virtual std::string getSuggestedAction() = 0;
};

#endif
