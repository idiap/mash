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


/** @file   classifier_input_set_listener_interface.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'IClassifierInputSetListener' interface
*/

#ifndef _MASH_ICLASSIFIERINPUTSETLISTENER_H_
#define _MASH_ICLASSIFIERINPUTSETLISTENER_H_

#include "declarations.h"
#include <mash/heuristic.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Interface of the object used by the Input Set to report events
    //--------------------------------------------------------------------------
    class MASH_SYMBOL IClassifierInputSetListener
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~IClassifierInputSetListener() {}


        //_____ Events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called when some features have been computed
        ///
        /// @param  detection       Indicates if the experiment is an
        ///                         object-detection one
        /// @param  training        Indicates if we are in the training or test
        ///                         phase
        /// @param  image           Index of the image in the current set
        ///                         (training or test)
        /// @param  original_image  Original index of the image (in the database)
        /// @param  coords          Position of the ROI
        /// #param  roiExtent       Extent of the ROI
        /// @param  heuristic       Index of the heuristic
        /// @param  nbFeatures      Number of features computed
        /// @param  indexes         Indexes of the computed features
        /// @param  values          Values of the features
        //----------------------------------------------------------------------
        virtual void onFeaturesComputed(bool detection,
                                        bool training,
                                        unsigned int image,
                                        unsigned int original_image,
                                        const coordinates_t& coords,
                                        unsigned int roiExtent,
                                        unsigned int heuristic,
                                        unsigned int nbFeatures,
                                        unsigned int* indexes,
                                        scalar_t* values) = 0;
    };
}

#endif
