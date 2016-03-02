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


/** @file   planner_delegate.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'IPlannerDelegate' interface
*/

#ifndef _MASH_IPLANNERDELEGATE_H_
#define _MASH_IPLANNERDELEGATE_H_

#include "declarations.h"
#include "planner.h"


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Interface that must be implemented by the classes managing a
    ///         goal-planner
    //--------------------------------------------------------------------------
    class MASH_SYMBOL IPlannerDelegate
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        IPlannerDelegate() {}

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~IPlannerDelegate() {}


        //_____ Goal-planner management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Tell the delegate about the folder into which the
        ///         goal-planners plugins are located
        ///
        /// @param  strPath     Path of the folder containing the plugins
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool setPlannersFolder(const std::string& strPath) = 0;

        //----------------------------------------------------------------------
        /// @brief  Tell the delegate load a specific goal-planner plugin and
        ///         create an instance of corresponding goal-planner
        ///
        /// @param  strName         Name of the plugin
        /// @param  strModelFile    (Optional) Path to the model file to load
        /// @return                 'true' if successful
        //----------------------------------------------------------------------
        virtual bool loadPlannerPlugin(const std::string& strName,
                                       const std::string& strModelFile = "") = 0;

        //----------------------------------------------------------------------
        /// @brief  Sets the notifier object to use
        //----------------------------------------------------------------------
        virtual void setNotifier(INotifier* pNotifier) = 0;


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Sets the initial seed that must be used by the goal-planner
        ///
        /// @param  seed    The initial seed
        //----------------------------------------------------------------------
        virtual bool setSeed(unsigned int seed) = 0;
        
        //----------------------------------------------------------------------
        /// @brief  Initialize the goal-planner
        ///
        /// @param  parameters  The goal-planner-specific parameters
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool setup(const tExperimentParametersList& parameters) = 0;

        //----------------------------------------------------------------------
        /// @brief  Load the model
        ///
        /// @param  perception  The (read-only) perception object to use to
        ///                     access the heuristics
        /// @return             'true' if successful
        ///
        /// @note   A model file must have been provided to the
        ///         loadPlannerPlugin() method
        //----------------------------------------------------------------------
        virtual bool loadModel(IPerception* perception) = 0;

        //----------------------------------------------------------------------
        /// @brief  Learn to solve the given task
        ///
        /// @param  task    The task to solve
        /// @return         'false' if an error occured
        //----------------------------------------------------------------------
        virtual bool learn(ITask* task) = 0;

        //----------------------------------------------------------------------
        /// @brief  Chooses an action from the given perception
        ///
        /// @param  perception  The perception of the task
        /// @retval action      The next action to perform
        /// @return             'false' if an error occured
        ///
        /// This method will be used for evaluation.
        ///
        /// You can safely assume that the method will only be called after
        /// learn().
        //----------------------------------------------------------------------
        virtual bool chooseAction(IPerception* perception, unsigned int* action) = 0;

        //----------------------------------------------------------------------
        /// @brief  Populates the provided list with the features used by the
        ///         goal-planner
        ///
        /// @param  perception  The (read-only) perception object to use to
        ///                     access the heuristics
        /// @retval list        The list of features used
        /// @return             'true' if successful
        ///
        /// You can safely assume that the method will only be called after
        /// learn(), and that the 'list' parameter is empty.
        //----------------------------------------------------------------------
        virtual bool reportFeaturesUsed(IPerception* perception, tFeatureList &list) = 0;

        //----------------------------------------------------------------------
        /// @brief  Save the model trained by the goal-planner
        ///
        /// @return 'true' if successful
        ///
        /// @note   This method will only be called at the end of the experiment.
        ///         The model must be saved in the Data Report, with the name
        ///         'predictor.model'.
        //----------------------------------------------------------------------
        virtual bool saveModel() = 0;
    
        //----------------------------------------------------------------------
        /// @brief  Returns the last error that occured
        //----------------------------------------------------------------------
        virtual tError getLastError() = 0;
    };
}

#endif
