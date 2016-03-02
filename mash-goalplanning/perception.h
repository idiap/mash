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


/** @file   perception.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Perception' class
*/

#ifndef _MASH_PERCEPTION_H_
#define _MASH_PERCEPTION_H_

#include "declarations.h"
#include "perception_interface.h"
#include "perception_listener_interface.h"
#include "task_controller.h"
#include <mash/features_computer.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Concrete implementation of the Perception (see IPerception)
    //--------------------------------------------------------------------------
    class MASH_SYMBOL Perception: public IPerception
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        Perception();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~Perception();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Retrieves the task controller object used
        ///
        /// @return The task controller object used
        //----------------------------------------------------------------------
        inline TaskController* getController()
        {
            return &_controller;
        }

        //----------------------------------------------------------------------
        /// @brief  Setup the perception
        //----------------------------------------------------------------------
        void setup(int view_size, int roi_extent);

        //----------------------------------------------------------------------
        /// @brief  Set the listener object to use to report events
        //----------------------------------------------------------------------
        inline void setListener(IPerceptionListener* pListener)
        {
            _pListener = pListener;
        }

        //----------------------------------------------------------------------
        /// @brief  Enable/disable the computation of features
        //----------------------------------------------------------------------
        inline void setReadOnly(bool readOnly)
        {
            _bReadOnly = readOnly;
        }


        //_____ Heuristics-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of heuristics
        //----------------------------------------------------------------------
        virtual unsigned int nbHeuristics()
        {
            return _computer.nbHeuristics();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of features (dimension) of a heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The dimension of the heuristic, 0 if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual unsigned int nbFeatures(unsigned int heuristic)
        {
            assert(_computer.initialized());
            return _computer.nbFeatures(heuristic);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the total number of features of the set
        //----------------------------------------------------------------------
        virtual unsigned int nbFeaturesTotal()
        {
            assert(_computer.initialized());
            return _computer.nbFeaturesTotal();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the name of a heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The name of the heuristic, empty if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual std::string heuristicName(unsigned int heuristic)
        {
            return _computer.heuristicsSet()->heuristicName(heuristic);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the seed of a heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The seed of the heuristic
        //----------------------------------------------------------------------
        virtual unsigned int heuristicSeed(unsigned int heuristic)
        {
            return _computer.heuristicSeed(heuristic);
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the given heuristic was referenced by the
        ///         goal-planner model that was loaded
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             'true' if the heuristic was in the model. When
        ///                     no model is used, 'false'.
        //----------------------------------------------------------------------
        virtual bool isHeuristicUsedByModel(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Mark a heuristic as used by the classifier model that was
        ///         loaded
        ///
        /// @param  heuristic   Index of the heuristic
        //----------------------------------------------------------------------
        void markHeuristicAsUsedByModel(unsigned int heuristic)
        {
            return _heuristicsInModel.push_back(heuristic);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns a pointer to the heuristics set used
        //----------------------------------------------------------------------
        inline FeaturesComputer* featuresComputer()
        {
            return &_computer;
        }


        //_____ Views-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of views
        //----------------------------------------------------------------------
        virtual unsigned int nbViews()
        {
            return _controller.nbViews();
        }

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
                                         scalar_t* values);

        //----------------------------------------------------------------------
        /// @brief  Returns the dimensions of the specified view
        ///
        /// @param  view    Index of the view
        //----------------------------------------------------------------------
        virtual dim_t viewSize(unsigned int view);

        //----------------------------------------------------------------------
        /// @brief  Returns the extent of the region of interest
        ///
        /// @remark The extent of the region of interest will never change
        ///         during an experiment
        //----------------------------------------------------------------------
        virtual unsigned int roiExtent()
        {
            return _roi_extent;
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if this is the beginning of a new sequence
        //----------------------------------------------------------------------
        virtual bool newSequence() const
        {
            return _newSequence;
        }

        inline unsigned int currentSequence() const
        {
            return _currentSequence;
        }

        inline unsigned int currentFrames() const
        {
            return _currentFrames;
        }

        void _onStateUpdated();
        void _onStateReset();
        Image* _getView(unsigned int view);


         //_____ Attributes __________
     protected:
         FeaturesComputer           _computer;
         TaskController             _controller;
         unsigned int               _roi_extent;
         unsigned int               _viewSize;
         unsigned int               _currentSequence;
         unsigned int               _currentFrames;
         bool                       _newSequence;
         Image**                    _views;
         IPerceptionListener*       _pListener;
         bool                       _bReadOnly;
         std::vector<unsigned int>  _heuristicsInModel;
    };
}

#endif
