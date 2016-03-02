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


/** Author: Philip Abbet (philip.abbet@idiap.ch)

    This goal-planner test the trajectories-related features of the
    goal-planning API
*/

#include <mash-goalplanning/planner.h>

using namespace Mash;
using namespace std;


//------------------------------------------------------------------------------
// Declaration of the goal-planner class
//------------------------------------------------------------------------------
class Trajectories: public Planner
{
    //_____ Construction / Destruction __________
public:
    Trajectories();
    virtual ~Trajectories();


    //_____ Methods to implement __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Initialize the goal-planner
    ///
    /// @param  parameters  The goal-planner-specific parameters
    /// @return             'true' if successful
    //--------------------------------------------------------------------------
    virtual bool setup(const tExperimentParametersList& parameters);

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
    virtual bool loadModel(PredictorModel &model);

    //--------------------------------------------------------------------------
    /// @brief  Learn to solve the given task
    ///
    /// @param  task    The task to solve
    /// @return         'false' if an error occured
    //--------------------------------------------------------------------------
    virtual bool learn(ITask* task);

    //--------------------------------------------------------------------------
    /// @brief  Chooses an action from the given perception
    ///
    /// @param  perception  The perception of the task
    /// @return             The next action to perform
    ///
    /// This method will be used for evaluation.
    ///
    /// You can safely assume that the method will only be called after
    /// learn().
    //--------------------------------------------------------------------------
    virtual unsigned int chooseAction(IPerception* perception);

    //--------------------------------------------------------------------------
    /// @brief  Populates the provided list with the features used by the
    ///         goal-planner
    ///
    /// @retval list    The list of features used
    /// @return         'true' if successful
    ///
    /// You can safely assume that the method will only be called after
    /// learn(), and that the 'list' parameter is empty.
    //--------------------------------------------------------------------------
    virtual bool reportFeaturesUsed(tFeatureList &list);

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
    virtual bool saveModel(PredictorModel &model);
};


//------------------------------------------------------------------------------
// Creation function of the goal-planner
//------------------------------------------------------------------------------
extern "C" Planner* new_planner()
{
    return new Trajectories();
}



/************************* CONSTRUCTION / DESTRUCTION *************************/

Trajectories::Trajectories()
{
}


Trajectories::~Trajectories()
{
}


/************************* IMPLEMENTATION OF Classifier ***********************/

bool Trajectories::setup(const tExperimentParametersList& parameters)
{
    return true;
}


bool Trajectories::loadModel(PredictorModel &model)
{
    return true;
}


bool Trajectories::learn(ITask* task)
{
    // Ensure that we are in a mode using recorded trajectories
    tGoalPlanningMode mode = task->mode();
    if ((mode != GPMODE_RECORDED_TEACHER) && (mode != GPMODE_RECORDED_TRAJECTORIES))
    {
        outStream << "ERROR: Not in a mode using recorded trajectories" << endl;
        return false;
    }

    // Retrieve the number of trajectories
    unsigned int nbTrajectories = task->nbTrajectories();
    if (nbTrajectories == 0)
    {
        outStream << "ERROR: No trajectory found" << endl;
        return false;
    }

    outStream << nbTrajectories << " trajectories found" << endl << endl;

    // Walk along all trajectories once
    for (unsigned int trajectory = 0; trajectory < nbTrajectories; ++trajectory)
    {
        outStream << "Processing trajectory #" << trajectory << endl;

        unsigned int length = task->trajectoryLength(trajectory);
        if (length == 0)
        {
            outStream << endl << "ERROR: Empty trajectory" << endl;
            return false;
        }

        outStream << "    Length: " << length << endl;

        for (unsigned int step = 0; step < length; ++step)
        {
            unsigned int action = task->suggestedAction();
            
            outStream << "    Action #" << step << ": " << action << endl;
            
            scalar_t reward = 0.0f;
            if (!task->performAction(action, &reward))
            {
                outStream << endl << "ERROR: Failed to perform the action" << endl;
                return false;
            }

            outStream << "        Reward: " << reward << endl;

            if ((task->result() != RESULT_NONE) && (step < length - 1))
            {
                outStream << endl << "ERROR: End of task reported before the end of the trajectory" << endl;
                return false;
            }
        }

        if (task->result() == RESULT_GOAL_REACHED)
            outStream << "    FINISHED" << endl;
        else if (task->result() == RESULT_TASK_FAILED)
            outStream << "    FAILED" << endl;

        outStream << endl;
        
        if (!task->reset())
        {
            outStream << "ERROR: Failed to reset the task" << endl;
            return false;
        }
    }

    outStream << "API successfully tested" << endl;
    
    return true;
}


unsigned int Trajectories::chooseAction(IPerception* perception)
{
    return 0;
}


bool Trajectories::reportFeaturesUsed(tFeatureList &list)
{
    return true;
}


bool Trajectories::saveModel(PredictorModel &model)
{
    return true;
}
