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


/** @file   classifier_input_set_listener.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'PerceptionListener' class
*/

#ifndef _MASH_PERCEPTIONLISTENER_H_
#define _MASH_PERCEPTIONLISTENER_H_

#include "instruments_set_interface.h"
#include <mash-goalplanning/perception_listener_interface.h>
#include <assert.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Implementation of the interface of the object used by the Input
    ///         Set to report events
    //--------------------------------------------------------------------------
    class MASH_SYMBOL PerceptionListener: public IPerceptionListener
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        PerceptionListener()
        : pInstrumentsSet(0)
        {
        }
        
        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~PerceptionListener()
        {
        }


        //_____ Events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called when some features have been computed
        ///
        /// @param  sequence    Index of the sequence
        /// @param  view        Index of the view
        /// @param  frame       Index of the frame
        /// @param  coords      Position of the ROI
        /// #param  roiExtent   Extent of the ROI
        /// @param  heuristic   Index of the heuristic
        /// @param  nbFeatures  Number of features computed
        /// @param  indexes     Indexes of the computed features
        /// @param  values      Values of the features
        //----------------------------------------------------------------------
        virtual void onFeaturesComputed(unsigned int sequence,
                                        unsigned int view,
                                        unsigned int frame,
                                        const coordinates_t& coords,
                                        unsigned int roiExtent,
                                        unsigned int heuristic,
                                        unsigned int nbFeatures,
                                        unsigned int* indexes,
                                        scalar_t* values)
        {
            assert(pInstrumentsSet);

            pInstrumentsSet->onFeaturesComputedByPlanner(sequence, view, frame,
                                                         coords, roiExtent,
                                                         heuristic, nbFeatures,
                                                         indexes, values);
        }
        
    
        //_____ Attributes __________
    public:
        IInstrumentsSet* pInstrumentsSet;
    };
}

#endif
