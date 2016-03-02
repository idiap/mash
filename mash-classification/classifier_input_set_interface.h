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


/** @file   classifier_input_set_interface.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'IClassifierInputSet' interface
*/

#ifndef _MASH_ICLASSIFIERINPUTSET_H_
#define _MASH_ICLASSIFIERINPUTSET_H_

#include "declarations.h"
#include <mash/heuristic.h>
#include <string>
#include <vector>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Interface of the object used by the classifiers to compute
    ///         features on the images
    ///
    /// To implement a classifier, you can safely assume that:
    ///   - The extent of the region of interest will never change during an
    ///     experiment
    ///   - (Images classification) All the images have the same size
    //--------------------------------------------------------------------------
    class MASH_SYMBOL IClassifierInputSet
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~IClassifierInputSet() {}


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the ID of the Inputs Set
        ///
        /// Each time something change in the Set (like the available images),
        /// the ID is changed too.
        //----------------------------------------------------------------------
        virtual unsigned int id() = 0;
        
        //----------------------------------------------------------------------
        /// @brief  Indicates if the task is an object detection one
        //----------------------------------------------------------------------
        virtual bool isDoingDetection() const = 0;
		

        //_____ Heuristics-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of heuristics in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbHeuristics() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the number of features (dimension) of a heuristic of
        ///         the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The dimension of the heuristic, 0 if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual unsigned int nbFeatures(unsigned int heuristic) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the total number of features of the set
        //----------------------------------------------------------------------
        virtual unsigned int nbFeaturesTotal() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the name of a heuristic of the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The name of the heuristic, empty if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual std::string heuristicName(unsigned int heuristic) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the seed of a heuristic of the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The seed of the heuristic
        //----------------------------------------------------------------------
        virtual unsigned int heuristicSeed(unsigned int heuristic) = 0;

        //----------------------------------------------------------------------
        /// @brief  Indicates if the given heuristic was referenced by the
        ///         classifier model that was loaded
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             'true' if the heuristic was in the model. When
        ///                     no model is used, 'false'.
        //----------------------------------------------------------------------
        virtual bool isHeuristicUsedByModel(unsigned int heuristic) = 0;


        //_____ Images-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of images in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbImages() = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the number of labels in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbLabels() = 0;

        //----------------------------------------------------------------------
        /// @brief  Computes several features of the specified heuristic on the
        ///         region of interest centered on a given point of an image
        ///
        /// @param  image       Index of the image
        /// @param  coordinates Coordinates of the center of the region of
        ///                     interest
        /// @param  heuristic   Index of the heuristic
        /// @param  nbFeatures  Number of features to compute
        /// @param  indexes     Indexes of the features to compute
        /// @param  values[out] The computed features
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool computeSomeFeatures(unsigned int image,
                                         const coordinates_t& coordinates,
                                         unsigned int heuristic,
                                         unsigned int nbFeatures,
                                         unsigned int* indexes,
                                         scalar_t* values) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the list of the objects in the specified image
        ///
        /// @param  image   Index of the image
        /// @retval objects The list of objects (with their labels) found in the
        ///                 image
        //----------------------------------------------------------------------
        virtual void objectsInImage(unsigned int image, tObjectsList* objects) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the list of negatives (positions without any objects)
        ///         in the specified image
        ///
        /// @param  image           Index of the image
        /// @param  positions[out]  The positions of the computed negatives
        //----------------------------------------------------------------------
        virtual void negativesInImage(unsigned int image,
                                      tCoordinatesList* positions) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the dimensions of the specified image
        ///
        /// @param  image   Index of the image
        ///
        /// @remark For images classification, all the images have the same size
        //----------------------------------------------------------------------
        virtual dim_t imageSize(unsigned int image) = 0;

        //----------------------------------------------------------------------
        /// @brief  Indicates if the image is part of the 'test set'
        ///
        /// @param  image   Index of the image
        ///
        /// @remark Only useful for the instruments, which can have access to
        ///         the full list of images
        //----------------------------------------------------------------------
        virtual bool isImageInTestSet(unsigned int image) = 0;

        //----------------------------------------------------------------------
        /// @brief  Returns the extent of the region of interest
        ///
        /// @remark The extent of the region of interest will never change
        ///         during an experiment
        //----------------------------------------------------------------------
        virtual unsigned int roiExtent() = 0;
    };
}

#endif
