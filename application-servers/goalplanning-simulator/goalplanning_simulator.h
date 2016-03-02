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


/** @file   goalplanning_simulator.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'GoalPlanningSimulator' class
*/

#ifndef _GOALPLANNING_SIMULATOR_H_
#define _GOALPLANNING_SIMULATOR_H_

#include <mash-appserver/application_server_interface.h>
#include <mash-utils/random_number_generator.h>
#include "logic_interface.h"


//------------------------------------------------------------------------------
/// @brief Concrete implementation of the Goal-planning Simulator
//------------------------------------------------------------------------------
class GoalPlanningSimulator: public Mash::IApplicationServer
{
    //_____ Internal types __________
public:
    struct tEnvironment
    {
        std::string name;
        std::string url;
        bool        recording;
    };

    typedef std::vector<tEnvironment>  tEnvironmentList;
    typedef tEnvironmentList::iterator tEnvironmentIterator;

    typedef std::map<std::string, tEnvironmentList> tTaskList;
    typedef tTaskList::iterator                     tTaskIterator;


    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief Constructor
    //--------------------------------------------------------------------------
    GoalPlanningSimulator();

    //--------------------------------------------------------------------------
    /// @brief Destructor
    //--------------------------------------------------------------------------
    virtual ~GoalPlanningSimulator();


    //_____ Static methods __________
public:
    static IApplicationServer* create();


    //_____ Methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief Sets the global seed
    //--------------------------------------------------------------------------
    virtual void setGlobalSeed(unsigned int seed);
    
    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the name of the available goals
    //--------------------------------------------------------------------------
    virtual Mash::tStringList getGoals();

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the name of the environment
    ///        suitable for the given goal
    ///
    /// @param goal         The name of the goal
    //--------------------------------------------------------------------------
    virtual Mash::tStringList getEnvironments(const std::string& goal);

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the possible actions in a task
    ///
    /// @param goal         The name of the goal
    /// @param environment  The name of the environment
    //--------------------------------------------------------------------------
    virtual Mash::tStringList getActions(const std::string& goal,
                                         const std::string& environment);

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the available views in a task
    ///
    /// @param goal         The name of the goal
    /// @param environment  The name of the environment
    //--------------------------------------------------------------------------
    virtual tViewsList getViews(const std::string& goal,
                                const std::string& environment);

    //--------------------------------------------------------------------------
    /// @brief Returns the mode of a task
    ///
    /// @param goal         The name of the goal
    /// @param environment  The name of the environment
    //--------------------------------------------------------------------------
    virtual Mash::tGoalPlanningMode getMode(const std::string& goal,
                                            const std::string& environment);

    //--------------------------------------------------------------------------
    /// @brief Initialize a task
    ///
    /// @param goal         The name of the goal
    /// @param environment  The name of the environment
    /// @param settings     Task-specific settings
    ///
    /// @remark Might be called several times. Cleanup the previous task as
    ///         needed.
    //--------------------------------------------------------------------------
    virtual bool initializeTask(const std::string& goal,
                                const std::string& environment,
                                const tSettingsList& settings);
    
    //--------------------------------------------------------------------------
    /// @brief Reset the task in its initial state
    ///
    /// @return 'true' if successful
    //--------------------------------------------------------------------------
    virtual bool resetTask();
    
    //--------------------------------------------------------------------------
    /// @brief Returns the number of trajectories available
    ///
    /// Only valid in GPMODE_RECORDED_TEACHER and GPMODE_RECORDED_TRAJECTORIES
    /// modes
    //--------------------------------------------------------------------------
    virtual unsigned int getNbTrajectories();

    //--------------------------------------------------------------------------
    /// @brief Returns the number of actions in a trajectory
    ///
    /// @param trajectory   Index of the trajectory
    ///
    /// Only valid in GPMODE_RECORDED_TEACHER and GPMODE_RECORDED_TRAJECTORIES
    /// modes
    //--------------------------------------------------------------------------
    virtual unsigned int getTrajectoryLength(unsigned int trajectory);

    //--------------------------------------------------------------------------
    /// @brief Returns one of the views
    ///
    /// @param[in]  view        The name of the view
    /// @param[out] nbBytes     The size of the returned data buffer, in
    ///                         bytes
    /// @param[out] mimetype    The MIME type of the data buffer (if 'raw',
    ///                         assume a RGB buffer with the dimensions of
    ///                         the view)
    /// @return                 Pointer to the data buffer, 0 in case of
    ///                         error (must be released by the caller)
    //--------------------------------------------------------------------------
    virtual unsigned char* getView(const std::string& view, size_t &nbBytes,
                                   std::string &mimetype);
    
    //--------------------------------------------------------------------------
    /// @brief Performs an action
    ///
    /// @param      action          The name of the action to perform
    /// @param[out] reward          The reward
    /// @param[out] finished        'true' if the goal was reached
    /// @param[out] failed          'true' if the goal isn't reacheable anymore
    /// @param[out] event           Human-readable description of what happened
    ///                             (optional)
    /// @return                     'false' in case of error
    //--------------------------------------------------------------------------
    virtual bool performAction(const std::string& action, float &reward,
                               bool &finished, bool &failed, std::string &event);

    //--------------------------------------------------------------------------
    /// @brief Returns the action that must be suggested to the client
    ///
    /// The meaning of 'suggestedAction' is dependent of the mode:
    ///   - GPMODE_STANDARD:                Not available
    ///   - GPMODE_TEACHER:                 Teacher action
    ///   - GPMODE_RECORDED_TEACHER:        Mandatory teacher action
    ///   - GPMODE_RECORDED_TRAJECTORIES:   Mandatory action
    //--------------------------------------------------------------------------
    virtual std::string getSuggestedAction();


protected:
    bool getEnvironment(const std::string& goal, const std::string& environment,
                        tEnvironment* pEnvironment);
    

    //_____ Static attributes __________
public:
    static tTaskList TASKS;


    //_____ Attributes __________
protected:
    ILogic*                     _logic;
    Mash::RandomNumberGenerator _generator;
    tEnvironment                _environment;
};

#endif
