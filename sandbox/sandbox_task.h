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


/** @file   sandbox_task.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxTask' class
*/

#ifndef _MASH_SANDBOXTASK_H_
#define _MASH_SANDBOXTASK_H_

#include <mash-goalplanning/declarations.h>
#include <mash-goalplanning/task_interface.h>
#include <mash-sandboxing/communication_channel.h>
#include <mash-utils/outstream.h>
#include "sandbox_perception.h"


//------------------------------------------------------------------------------
/// @brief  Implementation of the Task used by the sandboxed goal-planners
///         to retrieve data from the real Task object, located in the
///         calling process
//------------------------------------------------------------------------------
class MASH_SYMBOL SandboxTask: public Mash::ITask
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    ///
    /// @param  channel         The communication channel to use to communicate
    ///                         with the calling process
    /// @param  pOutStream      The output stream to use (can be 0)
    /// @param  pWardenContext  The warden context to use
    /// @param  bReadOnly       Indicates if the task is in read-only mode
    //--------------------------------------------------------------------------
    SandboxTask(const Mash::CommunicationChannel& channel,
                Mash::OutStream* pOutStream,
                tWardenContext* pWardenContext,
                bool bReadOnly = false);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~SandboxTask();


    //_____ Methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Returns the perception of the task
    //--------------------------------------------------------------------------
    virtual Mash::IPerception* perception()
    {
        return &_perception;
    }

    //--------------------------------------------------------------------------
    /// @brief  Returns the mode of the task
    //--------------------------------------------------------------------------
    virtual Mash::tGoalPlanningMode mode();

    //--------------------------------------------------------------------------
    /// @brief  Returns the number of available actions
    //--------------------------------------------------------------------------
    virtual unsigned int nbActions();

    //--------------------------------------------------------------------------
    /// @brief  Returns the number of available trajectories
    ///
    /// Only available in GPMODE_RECORDED_TEACHER and
    /// GPMODE_RECORDED_TRAJECTORIES modes
    //--------------------------------------------------------------------------
    virtual unsigned int nbTrajectories();

    //--------------------------------------------------------------------------
    /// @brief  Returns the length of a given trajectory
    ///
    /// @param trajectory   Index of the trajectory
    ///
    /// Only available in GPMODE_RECORDED_TEACHER and
    /// GPMODE_RECORDED_TRAJECTORIES modes
    //--------------------------------------------------------------------------
    virtual unsigned int trajectoryLength(unsigned int trajectory);

    //--------------------------------------------------------------------------
    /// @brief  Puts the task in an initial state
    ///
    /// @return 'true' if successfull
    ///
    /// @remark The initial state is task-dependant: it can be something
    ///         random or always the same thing
    //--------------------------------------------------------------------------
    virtual bool reset();

    //--------------------------------------------------------------------------
    /// @brief  Perform the specified action
    ///
    /// @param  action  The action
    /// @retval reward  The reward
    /// @return         'true' if successful
    //--------------------------------------------------------------------------
    virtual bool performAction(unsigned int action, Mash::scalar_t* reward);

    //--------------------------------------------------------------------------
    /// @brief  Indicates if the goal was reached, if the task was failed,
    ///         ...
    /// @return The result of the task
    //--------------------------------------------------------------------------
    virtual Mash::tResult result()
    {
        return _result;
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
    virtual unsigned int suggestedAction();


    //_____ Communication-related methods __________
public:
    bool waitResponse();


     //_____ Attributes __________
 protected:
     Mash::CommunicationChannel _channel;
     Mash::OutStream            _outStream;
     tWardenContext*            _pWardenContext;
     bool                       _bReadOnly;
     SandboxPerception          _perception;
     Mash::tGoalPlanningMode    _mode;
     unsigned int               _nbActions;
     std::vector<unsigned int>  _trajectoryLengths;
     Mash::tResult              _result;
     int                        _suggestedAction;
};

#endif
