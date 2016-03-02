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


/** @file   perception_interface.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'IPerception' interface
*/

#ifndef _MASH_PERCEPTION_INTERFACE_H_
#define _MASH_PERCEPTION_INTERFACE_H_

#include "declarations.h"
#include <mash/heuristic.h>


namespace Mash
{
    typedef float scalar_t;


    //--------------------------------------------------------------------------
    /// @brief  Interface of the object used by the goal-planners to compute
    ///         features on the views
    ///
    /// To implement a goal-planner, you can safely assume that the number of
    /// views doesn't change
    //--------------------------------------------------------------------------
    class MASH_SYMBOL IPerception
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~IPerception() {}


        //_____ Heuristics-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of heuristics
        //----------------------------------------------------------------------
        virtual unsigned int nbHeuristics() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the number of features (dimension) of a heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The dimension of the heuristic, 0 if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual unsigned int nbFeatures(unsigned int heuristic) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the total number of features in the perception
        //----------------------------------------------------------------------
        virtual unsigned int nbFeaturesTotal() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the name of a heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The name of the heuristic, empty if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual std::string heuristicName(unsigned int heuristic) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the seed of a heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The seed of the heuristic
        //----------------------------------------------------------------------
        virtual unsigned int heuristicSeed(unsigned int heuristic) = 0;

        //----------------------------------------------------------------------
        /// @brief  Indicates if the given heuristic was referenced by the
        ///         goal-planner model that was loaded
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             'true' if the heuristic was in the model. When
        ///                     no model is used, 'false'.
        //----------------------------------------------------------------------
        virtual bool isHeuristicUsedByModel(unsigned int heuristic) = 0;


        //_____ Views-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of views
        //----------------------------------------------------------------------
        virtual unsigned int nbViews() = 0;

        //----------------------------------------------------------------------
        /// @brief  Computes several features of the specified heuristic on the
        ///         current frame of the specified view
        ///
        /// @param  view        Index of the view
        /// @param  coordinates Coordinates of the center of the region of
        ///                     interest
        /// @param  heuristic   Index of the heuristic
        /// @param  nbFeatures  Number of features to compute
        /// @param  indexes     Indexes of the features to compute
        /// @param  values[out] The computed features
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool computeSomeFeatures(unsigned int view,
                                         const coordinates_t& coordinates,
                                         unsigned int heuristic,
                                         unsigned int nbFeatures,
                                         unsigned int* indexes,
                                         scalar_t* values) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the dimensions of the specified view
        ///
        /// @param  view    Index of the view
        //----------------------------------------------------------------------
        virtual dim_t viewSize(unsigned int view) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the extent of the region of interest
        ///
        /// @remark The extent of the region of interest will never change
        ///         during an experiment
        //----------------------------------------------------------------------
        virtual unsigned int roiExtent() = 0;

        //----------------------------------------------------------------------
        /// @brief  Indicates if this is the beginning of a new sequence
        //----------------------------------------------------------------------
        virtual bool newSequence() const = 0;
    };
}

#endif
