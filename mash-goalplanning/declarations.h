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


/** @file   declarations.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declarations used all over mash-goalplanning
*/

#ifndef _MASHGOALPLANNING_DECLARATIONS_H_
#define _MASHGOALPLANNING_DECLARATIONS_H_

#include <mash-utils/declarations.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  The possible results of a goal-planning task
    //--------------------------------------------------------------------------
    enum tResult
    {
        RESULT_NONE,            ///< No result yet, additional actions can be performed
        RESULT_GOAL_REACHED,    ///< The goal was reached
        RESULT_TASK_FAILED,     ///< The task was failed
    };
}

#endif
