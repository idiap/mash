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


/** @file   task.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Task' class
*/

#ifndef _MASH_TASK_H_
#define _MASH_TASK_H_

#include "task_interface.h"
#include "perception.h"


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Concrete implementation of the Task (see ITask)
    //--------------------------------------------------------------------------
    class MASH_SYMBOL Task: public ITask
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        Task();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~Task();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Set the folder into which the images received from the
        ///         Application Server must be written
        //----------------------------------------------------------------------
        void setCaptureFolder(const std::string& strCaptureFolder);

        //----------------------------------------------------------------------
        /// @brief  Enable/disable the execution of actions
        //----------------------------------------------------------------------
        inline void setReadOnly(bool readOnly)
        {
            _bReadOnly = readOnly;
            _perception.setReadOnly(readOnly);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the perception of the task
        //----------------------------------------------------------------------
        virtual IPerception* perception()
        {
            return &_perception;
        }

        //----------------------------------------------------------------------
        /// @brief  Retrieves the task controller object used
        ///
        /// @return The task controller object used
        //----------------------------------------------------------------------
        inline TaskController* getController()
        {
            return _perception.getController();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the mode of the task
        //----------------------------------------------------------------------
        virtual tGoalPlanningMode mode()
        {
            return _perception.getController()->mode();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of available actions
        //----------------------------------------------------------------------
        virtual unsigned int nbActions()
        {
            return _perception.getController()->nbActions();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of available trajectories
        ///
        /// Only available in GPMODE_RECORDED_TEACHER and
        /// GPMODE_RECORDED_TRAJECTORIES modes
        //----------------------------------------------------------------------
        virtual unsigned int nbTrajectories()
        {
            return _perception.getController()->nbTrajectories();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the length of a given trajectory
        ///
        /// @param trajectory   Index of the trajectory
        ///
        /// Only available in GPMODE_RECORDED_TEACHER and
        /// GPMODE_RECORDED_TRAJECTORIES modes
        //----------------------------------------------------------------------
        virtual unsigned int trajectoryLength(unsigned int trajectory)
        {
            return _perception.getController()->trajectoryLength(trajectory);
        }

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
        virtual bool reset();

        //----------------------------------------------------------------------
        /// @brief  Perform the specified action
        ///
        /// @param  action  The action
        /// @retval reward  The reward
        /// @return         'true' if successful
        //----------------------------------------------------------------------
        virtual bool performAction(unsigned int action, scalar_t* reward);

        //----------------------------------------------------------------------
        /// @brief  Indicates if the goal was reached, if the task was failed,
        ///         ...
        /// @return The result of the task
        //----------------------------------------------------------------------
        virtual tResult result()
        {
            return _perception.getController()->result();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the suggested action
        ///
        /// The meaning of 'suggestedAction' is dependent of the mode:
        ///   - GPMODE_STANDARD:                Not available
        ///   - GPMODE_TEACHER:                 Teacher action
        ///   - GPMODE_RECORDED_TEACHER:        Mandatory teacher action
        ///   - GPMODE_RECORDED_TRAJECTORIES:   Mandatory action
        //----------------------------------------------------------------------
        virtual unsigned int suggestedAction()
        {
            return _perception.getController()->suggestedAction();
        }


         //_____ Attributes __________
     protected:
         Perception         _perception;
         std::string        _strCaptureFolder;
         bool               _bReadOnly;
    };
}

#endif
