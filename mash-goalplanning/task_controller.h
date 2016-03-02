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


/** @file   task_controller.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'TaskController' class
*/

#ifndef _MASH_TASKCONTROLLER_H_
#define _MASH_TASKCONTROLLER_H_

#include "declarations.h"
#include "perception_interface.h"
#include <mash/image.h>
#include <mash-network/client.h>
#include <mash-utils/arguments_list.h>
#include <vector>
#include <assert.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Interface to an interactive Application Server
    //--------------------------------------------------------------------------
    class MASH_SYMBOL TaskController
    {
        //_____ Internal types __________
    private:
        struct tView
        {
            std::string     strName;
            unsigned int    width;
            unsigned int    height;
        };

        typedef std::vector<tView>          tViewsList;
        typedef tViewsList::iterator        tViewsIterator;


        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        TaskController();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~TaskController();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Set the client object to use to communicate with the
        ///         application server
        ///
        /// @param  pClient     Client object to use. Must be alread connected
        ///                     to an application server.
        /// @return             Error code
        ///
        /// @remark Takes ownership of the client object
        //----------------------------------------------------------------------
        tError setClient(Client* pClient);

        //----------------------------------------------------------------------
        /// @brief  Retrieves the client object used
        ///
        /// @return The client object used
        //----------------------------------------------------------------------
        inline Client* getClient()
        {
            return _pClient;
        }

        //----------------------------------------------------------------------
        /// @brief  Select the task to solve
        ///
        /// @param  strGoal         Name of the goal
        /// @param  strEnvironment  Name of the environment
        /// @param  parameters      Parameters of the task
        /// @return                 Error code
        //----------------------------------------------------------------------
        tError selectTask(const std::string& strGoal, const std::string& strEnvironment,
                          const tExperimentParametersList& parameters);

        //----------------------------------------------------------------------
        /// @brief  Reset the state of the task
        ///
        /// @return Error code
        ///
        /// @remark The initial state is task-dependant: it can be something
        ///         random or always the same thing
        //----------------------------------------------------------------------
        tError resetTask();

        //----------------------------------------------------------------------
        /// @brief  Returns the mode of the task
        //----------------------------------------------------------------------
        inline tGoalPlanningMode mode()
        {
            return _mode;
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the goal was reached, if the task was failed,
        ///         ...
        /// @return The result of the task
        //----------------------------------------------------------------------
        inline tResult result() const
        {
            return _result;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of available trajectories
        ///
        /// Only available in GPMODE_RECORDED_TEACHER and
        /// GPMODE_RECORDED_TRAJECTORIES modes
        //----------------------------------------------------------------------
        unsigned int nbTrajectories();

        //----------------------------------------------------------------------
        /// @brief  Returns the length of a given trajectory
        ///
        /// @param trajectory   Index of the trajectory
        ///
        /// Only available in GPMODE_RECORDED_TEACHER and
        /// GPMODE_RECORDED_TRAJECTORIES modes
        //----------------------------------------------------------------------
        unsigned int trajectoryLength(unsigned int trajectory);

        //----------------------------------------------------------------------
        /// @brief  Returns a description of the last ERROR_EXPERIMENT_PARAMETER
        ///         error that occured
        //----------------------------------------------------------------------
        inline std::string getLastError()
        {
            std::string ret = _strLastError;
            _strLastError = "";
            return ret;
        }


        //_____ Views-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of views
        //----------------------------------------------------------------------
        inline unsigned int nbViews() const
        {
            return (unsigned int) _views.size();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the specified view
        ///
        /// @param  index   Index of the view (from 0 to nbViews()-1)
        //----------------------------------------------------------------------
        Image* getView(unsigned int index);

        //----------------------------------------------------------------------
        /// @brief  Returns the dimensions of the specified view
        ///
        /// @param  index   Index of the view
        //----------------------------------------------------------------------
        inline dim_t viewSize(unsigned int index) const
        {
            assert(index < nbViews());

            dim_t size;
            size.width = _views[index].width;
            size.height = _views[index].height;
            return size;
        }

        //----------------------------------------------------------------------
        /// @brief  Suggest a ROI extent based on the size of the views
        //----------------------------------------------------------------------
        unsigned int suggestROIExtent();
        

        //_____ Actions-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of available actions
        //----------------------------------------------------------------------
        inline unsigned int nbActions() const
        {
            return (unsigned int) _actions.size();
        }

        //----------------------------------------------------------------------
        /// @brief  Perform the specified action
        ///
        /// @param  action          The action
        /// @retval reward          The reward
        /// @return                 Error code
        //----------------------------------------------------------------------
        tError performAction(unsigned int action, scalar_t* reward);

        //----------------------------------------------------------------------
        /// @brief  Returns the suggested action
        ///
        /// The meaning of 'suggestedAction' is dependent of the mode:
        ///   - GPMODE_STANDARD:                Not available
        ///   - GPMODE_TEACHER:                 Teacher action
        ///   - GPMODE_RECORDED_TEACHER:        Mandatory teacher action
        ///   - GPMODE_RECORDED_TRAJECTORIES:   Mandatory action
        //----------------------------------------------------------------------
        inline unsigned int suggestedAction()
        {
            return _suggestedAction;
        }


        //_____ Attributes __________
    private:
        Client*             _pClient;
        bool                _bSupportRecordedSequences;
        tStringList         _actions;
        tViewsList          _views;
        tGoalPlanningMode   _mode;
        unsigned int        _nbTrajectories;
        tResult             _result;
        unsigned int        _suggestedAction;
        std::string         _strLastError;
    };
}

#endif
