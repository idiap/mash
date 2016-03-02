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


/** @file   planner.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Planner' interface
*/

#ifndef _MASH_PLANNER_H_
#define _MASH_PLANNER_H_

#include "declarations.h"
#include "task_interface.h"
#include <mash-utils/outstream.h>
#include <mash-utils/random_number_generator.h>
#include <mash-utils/data_writer.h>
#include <mash/predictor_model.h>
#include <mash/notifier.h>
#include <vector>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Base class for the goal-planners
    //--------------------------------------------------------------------------
    class MASH_SYMBOL Planner
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        Planner() {}

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~Planner() {}


        //_____ Methods to implement __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Initialize the goal-planner
        ///
        /// @param  parameters  The goal-planner-specific parameters
        /// @return             'true' if successful
        ///
        /// @note   The 'tExperimentParametersList' type is defined in
        ///         'mash-utils/declarations.h', with an usage example
        //----------------------------------------------------------------------
        virtual bool setup(const tExperimentParametersList& parameters) = 0;

        //----------------------------------------------------------------------
        /// @brief  Load a model
        ///
        /// @param  model   The model object to use
        /// @return         'true' if successful
        ///
        /// @note   This method will be called after setup(), but only if an
        ///         already trained model must be used. Then, either we begin
        ///         directly with calls to chooseAction(), or the goal-planner
        ///         is given the opportunity to adapt its model via a call to
        ///         learn().
        ///
        /// The provided model has already been initialized (ie. it already
        /// knows about the heuristics used by the goal-planner that saved it).
        //----------------------------------------------------------------------
        virtual bool loadModel(PredictorModel &model) = 0;

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
        /// @return             The next action to perform
        ///
        /// This method will be used for evaluation.
        //----------------------------------------------------------------------
        virtual unsigned int chooseAction(IPerception* perception) = 0;

        //----------------------------------------------------------------------
        /// @brief  Populates the provided list with the features used by the
        ///         goal-planner
        ///
        /// @retval list    The list of features used
        /// @return         'true' if successful
        ///
        /// @note   This method will only be called after learn(), and the
        ///         'list' parameter will always be empty.
        ///
        /// @note   The 'tFeatureList' type is defined in
        ///         'mash-utils/declarations.h', with an usage example
        //----------------------------------------------------------------------
        virtual bool reportFeaturesUsed(tFeatureList &list) = 0;

        //----------------------------------------------------------------------
        /// @brief  Save the model trained by the goal-planner
        ///
        /// @param  model   The model object to use
        /// @return         'true' if successful
        ///
        /// @note   This method will only be called at the end of the experiment.
        ///
        /// The provided model has already been initialized (ie. it already
        /// knows about the heuristics used by the goal-planner, reported by
        /// reportFeaturesUsed()).
        //----------------------------------------------------------------------
        virtual bool saveModel(PredictorModel &model) = 0;


        //_____ Attributes __________
    public:
        OutStream               outStream;  ///< The stream to use for logging
        RandomNumberGenerator   generator;  ///< The random number generator to use
        Notifier                notifier;   ///< The object to use to send notifications
        DataWriter              writer;     ///< The object to use to save data to be
                                            ///  analyzed later
    };


    //--------------------------------------------------------------------------
    /// @brief  Prototype of the function used to create an instance of a
    ///         specific goal-planner object
    ///
    /// Each goal-planner must have an associated function called 'new_planner'
    /// that creates an instance of the goal-planner. For instance, if your
    /// goal-planner class is 'MyPlanner', your 'new_planner' function must be:
    /// @code
    /// extern "C" Planner* new_planner()
    /// {
    ///     return new MyPlanner();
    /// }
    /// @endcode
    //--------------------------------------------------------------------------
    typedef Planner* tPlannerConstructor();
}

#endif
