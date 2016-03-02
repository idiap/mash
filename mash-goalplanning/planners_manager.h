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


/** @file   planners_manager.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'PlannersManager' class
*/

#ifndef _MASH_PLANNERSMANAGER_H_
#define _MASH_PLANNERSMANAGER_H_

#include "declarations.h"
#include "planner.h"
#include <mash/dynlibs_manager.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Manage the loading/unloading of the goal-planners
    ///
    /// Each goal-planner is contained in its own dynamic library. This class
    /// manages the loading of these libraries, and the creation of instances of
    /// their goal-planners.
    //--------------------------------------------------------------------------
    class MASH_SYMBOL PlannersManager: public DynlibsManager
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param  strRootFolder   Path to the folder containing all the
        ///                         goal-planners (in dynamic libraries form) 
        //----------------------------------------------------------------------
        PlannersManager(const std::string& strRootFolder = "");

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~PlannersManager();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns a new instance of a goal-planner
        ///
        /// @param  strName Name of the goal-planner in one of the following
        ///                 forms: 'author/name/version', 'author/name' (if
        ///                 there is only one version) or 'name' (if the
        ///                 goal-planner isn't located in a user folder)
        /// @return         A new instance of the goal-planner
        ///
        /// @remark The first time the name is provided, the dynamic library
        ///         containing the goal-planner is loaded.
        //----------------------------------------------------------------------
        Planner* create(const std::string& strName);


    private:
        tPlannerConstructor* getPlannerConstructor(const std::string& strPlanner);
    };
}

#endif
