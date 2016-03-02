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


/** @file   task_interface.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ITask' interface
*/

#ifndef _MASH_TASK_INTERFACE_H_
#define _MASH_TASK_INTERFACE_H_

#include "declarations.h"
#include "perception_interface.h"


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Interface of the object used by the goal-planners to perform
    ///         actions
    ///
    /// To implement a goal-planner, you can safely assume that the number of
    /// actions doesn't change
    //--------------------------------------------------------------------------
    class MASH_SYMBOL ITask
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~ITask() {}


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the perception of the task
        //----------------------------------------------------------------------
        virtual IPerception* perception() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the mode of the task
        //----------------------------------------------------------------------
        virtual tGoalPlanningMode mode() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the number of available actions
        //----------------------------------------------------------------------
        virtual unsigned int nbActions() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the number of available trajectories
        ///
        /// Only available in GPMODE_RECORDED_TEACHER and
        /// GPMODE_RECORDED_TRAJECTORIES modes
        //----------------------------------------------------------------------
        virtual unsigned int nbTrajectories() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the length of a given trajectory
        ///
        /// @param trajectory   Index of the trajectory
        ///
        /// Only available in GPMODE_RECORDED_TEACHER and
        /// GPMODE_RECORDED_TRAJECTORIES modes
        //----------------------------------------------------------------------
        virtual unsigned int trajectoryLength(unsigned int trajectory) = 0;

        //----------------------------------------------------------------------
        /// @brief  Puts the task in an initial state
        ///
        /// @return 'true' if successfull
        ///
        /// @remark When the mode is GPMODE_STANDARD or GPMODE_TEACHER, the
        ///         initial state is task-dependent: it can be something random
        ///         or always the same thing.
        ///         When the mode is GPMODE_RECORDED_TEACHER or
        ///         GPMODE_RECORDED_TRAJECTORIES, the task switch to the next
        ///         trajectory.
        //----------------------------------------------------------------------
        virtual bool reset() = 0;

        //----------------------------------------------------------------------
        /// @brief  Perform the specified action
        ///
        /// @param  action  The action
        /// @retval reward  The reward
        /// @return         'true' if successful
        //----------------------------------------------------------------------
        virtual bool performAction(unsigned int action, scalar_t* reward) = 0;

        //----------------------------------------------------------------------
        /// @brief  Indicates if the goal was reached, if the task was failed,
        ///         ...
        /// @return The result of the task
        //----------------------------------------------------------------------
        virtual tResult result() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the suggested action
        ///
        /// The meaning of 'suggestedAction' is dependent of the mode:
        ///   - GPMODE_STANDARD:                Not available
        ///   - GPMODE_TEACHER:                 Teacher action
        ///   - GPMODE_RECORDED_TEACHER:        Mandatory teacher action
        ///   - GPMODE_RECORDED_TRAJECTORIES:   Mandatory action
        //----------------------------------------------------------------------
        virtual unsigned int suggestedAction() = 0;
    };
}

#endif
