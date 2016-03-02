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


/** @file   logic_2d_movements.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Logic2DMovements' class
*/

#ifndef _LOGIC_2D_MOVEMENTS_H_
#define _LOGIC_2D_MOVEMENTS_H_

#include <logic_interface.h>


//------------------------------------------------------------------------------
/// @brief Interface of an object defining the logic of a task
//------------------------------------------------------------------------------
class Logic2DMovements: public ILogic
{
    //_____ Internal types __________
public:
    struct tPosition
    {
        tPosition()
        : x(-1), y(-1)
        {
        }

        tPosition(int x, int y)
        : x(x), y(y)
        {
        }
        
        int x;
        int y;
    };

    typedef std::vector<tPosition>  tPositionList;
    typedef tPositionList::iterator tPositionIterator;


    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief Constructor
    //--------------------------------------------------------------------------
    Logic2DMovements();

    //--------------------------------------------------------------------------
    /// @brief Destructor
    //--------------------------------------------------------------------------
    virtual ~Logic2DMovements();


    //_____ Methods __________
public:
    inline void setSize(int width, int height)
    {
        _width = width;
        _height = height;
    }
    
    void setStart(const tPosition& pos)
    {
        _start = pos;
    }

    void setGoalOrObstacle(const tPosition& pos, bool goal)
    {
        if (goal)
            _goal = pos;
        else
            _obstacles.push_back(pos);
    }

protected:
    bool obstacleAtPosition(const tPosition& pos);


    //_____ Implementation of ILogic __________
public:
    //--------------------------------------------------------------------------
    /// @brief Returns the mode of the task
    //--------------------------------------------------------------------------
    virtual Mash::tGoalPlanningMode getMode()
    {
        return Mash::GPMODE_STANDARD;
    }

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the possible actions
    //--------------------------------------------------------------------------
    virtual Mash::tStringList getActions();

    //--------------------------------------------------------------------------
    /// @brief Returns a list containing the available views
    //--------------------------------------------------------------------------
    virtual Mash::IApplicationServer::tViewsList getViews();

    //--------------------------------------------------------------------------
    /// @brief Returns the number of trajectories available
    ///
    /// Only valid in GPMODE_RECORDED_TEACHER and GPMODE_RECORDED_TRAJECTORIES
    /// modes
    //--------------------------------------------------------------------------
    virtual unsigned int getNbTrajectories()
    {
        return 0;
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
        return 0;
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
        return "";
    }


    //_____ Attributes __________
protected:
    int             _width;
    int             _height;
    tPosition       _position;
    tPositionList   _obstacles;
    tPosition       _goal;
    tPosition       _start;
};

#endif
