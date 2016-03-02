/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Leonidas Lefakis (leonidas.lefakis@idiap.ch)
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


/** @file   goal_planning_classifier_input_set.h
    @author Leonidas Lefakis (leonidas.lefakis@idiap.ch)

    Declaration of the 'GPClassifierInputSet' class
*/

#ifndef _CLASSIFICATION_UTILS_GOALPLANNINGCLASSIFIERINPUTSET_H_
#define _CLASSIFICATION_UTILS_GOALPLANNINGCLASSIFIERINPUTSET_H_

#include <mash-goalplanning/perception_interface.h>
#include <mash-classification/declarations.h>
#include <mash-classification/classifier_input_set_interface.h>
#include <mash-classification/classifier_input_set_listener_interface.h>
#include <mash-utils/arguments_list.h>
#include <map>
#include <vector>
#include <algorithm>


namespace ClassificationUtils
{

    class GPClassifierInputSet: public Mash::IClassifierInputSet
    {

    public:

        GPClassifierInputSet(Mash::IPerception* perception);
        GPClassifierInputSet(Mash::IPerception* perception,unsigned int mNbF);
        virtual ~GPClassifierInputSet();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the ID of the Inputs Set
        ///
        /// Each time something change in the Set (like the available images),
        /// the ID is changed too.
        //----------------------------------------------------------------------
        virtual unsigned int id()
        {
            return _id;
        }

        //----------------------------------------------------------------------
        /// @brief  Must be called when the data in the set has changed
        //----------------------------------------------------------------------
        inline void onUpdated()
        {
            ++_id;
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the task is an object detection one
        //----------------------------------------------------------------------
        virtual bool isDoingDetection() const
        {
            return false;
        }


        //_____ Heuristics-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of heuristics in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbHeuristics()
        {
            return _nbHeuristics;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of features (dimension) of a heuristic of
        ///         the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The dimension of the heuristic, 0 if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual unsigned int nbFeatures(unsigned int heuristic)
        {
            return _nbFeatures[heuristic];
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the total number of features of the set
        //----------------------------------------------------------------------
        virtual unsigned int nbFeaturesTotal()
        {
            unsigned int _nbFeaturesTotal = 0;
            for (unsigned int i = 0 ; i<_nbFeatures.size(); ++i) _nbFeaturesTotal += _nbFeatures[i];
                return _nbFeaturesTotal;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the name of a heuristic of the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The name of the heuristic, empty if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        virtual std::string heuristicName(unsigned int heuristic)
        {
            return _heuristicName[heuristic];
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the seed of a heuristic of the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The seed of the heuristic
        //----------------------------------------------------------------------
        virtual unsigned int heuristicSeed(unsigned int heuristic)
        {
            return _heuristicSeed[heuristic];
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the given heuristic was referenced by the
        ///         classifier model that was loaded
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             'true' if the heuristic was in the model. When
        ///                     no model is used, 'false'.
        //----------------------------------------------------------------------
        virtual bool isHeuristicUsedByModel(unsigned int heuristic)
        {
            return _iHUBM[heuristic];
        }


        //_____ Images-related methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the number of images in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbImages()
        {
            return _nbImages;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of labels in the set
        //----------------------------------------------------------------------
        virtual unsigned int nbLabels()
        {
            return _nbLabels;
        }

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
                                         const Mash::coordinates_t& coordinates,
                                         unsigned int heuristic,
                                         unsigned int nbFeatures,
                                         unsigned int* indexes,
                                         Mash::scalar_t* values);

        //----------------------------------------------------------------------
        /// @brief  Returns the list of the objects in the specified image
        ///
        /// @param  image   Index of the image
        /// @retval objects The list of objects (with their labels) found in the
        ///                 image
        //----------------------------------------------------------------------
        virtual void objectsInImage(unsigned int image, Mash::tObjectsList* objects);


        //----------------------------------------------------------------------
        /// @brief  Returns the list of negatives (positions without any objects)
        ///         in the specified image
        ///
        /// @param  image           Index of the image
        /// @param  positions[out]  The positions of the computed negatives
        //----------------------------------------------------------------------
        virtual void negativesInImage(unsigned int image,
                                      Mash::tCoordinatesList* positions){}

        //----------------------------------------------------------------------
        /// @brief  Returns the dimensions of the specified image
        ///
        /// @param  image   Index of the image
        ///
        /// @remark For images classification, all the images have the same size
        //----------------------------------------------------------------------
        virtual Mash::dim_t imageSize(unsigned int image)
        {
            return _viewSize[image];
        }

        //----------------------------------------------------------------------
        /// @brief  Indicates if the image is part of the 'test set'
        ///
        /// @param  image   Index of the image
        ///
        /// @remark Only useful for the instruments, which can have access to
        ///         the full list of images
        //----------------------------------------------------------------------
        virtual bool isImageInTestSet(unsigned int image)
        {
            return false;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the extent of the region of interest
        ///
        /// @remark The extent of the region of interest will never change
        ///         during an experiment
        //----------------------------------------------------------------------
        virtual unsigned int roiExtent()
        {
            return _roiExtent;
        }


        //_____ Additional methods __________
    public:
    	unsigned int maxNbFeatures()
        {
            return _max_nbFeatures;
        }

    	unsigned int allocatedMemory() 
    	{
    	    return (1 + _data.size()) * (sizeof(Mash::dim_t) + sizeof(unsigned int) + sizeof(Mash::scalar_t) * nbFeaturesTotal());
    	}

        void pushData(const std::vector<Mash::scalar_t>& values,
                      Mash::dim_t viewSize, unsigned int label);


        void setNumLabels(unsigned int nbLabels)
        {
            _nbLabels = nbLabels;
        }


        //_____ Attributes __________
    protected:
        unsigned int _id;
        unsigned int _nbHeuristics;
        unsigned int _roiExtent;
        unsigned int _nbImages;
        unsigned int _nbLabels;
        unsigned int _max_nbFeatures;

        std::vector <std::vector <Mash::scalar_t> > _data;
        std::vector <unsigned int>                  _nbFeatures;
        std::vector <Mash::dim_t>                   _viewSize;
        std::vector <unsigned int>                  _heuristicSeed;
        std::vector <std::string>                   _heuristicName;
        std::vector <unsigned int>                  _Labels;
        std::vector<bool>                           _iHUBM;
    };
}

#endif
