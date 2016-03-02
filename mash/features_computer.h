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


/** @file   features_computer.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'FeaturesComputer' class
*/

#ifndef _MASH_FEATURESCOMPUTER_H_
#define _MASH_FEATURESCOMPUTER_H_

#include <mash-utils/declarations.h>
#include "images_cache.h"
#include "heuristics_set_interface.h"
#include <assert.h>
#include <vector>
#include <map>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represesents a Features Computer, used in an experiment to
    ///         efficiently compute the features
    ///
    /// The 'context' of each heuristic is stored by this class. This allows
    /// some optimizations when nothing or only a few things changed. Due to
    /// this mecanism, the Computer must be notified when one of the images used
    /// by the heuristics is removed from the memory. That's why it inherits
    /// from ImagesCache::IListener (don't forget to register it with the cache!).
    //--------------------------------------------------------------------------
    class MASH_SYMBOL FeaturesComputer: public ImagesCache::IListener
    {
        //_____ Internal types __________
    public:
        typedef std::vector<unsigned int> tFeaturesList;
        

        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        FeaturesComputer();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~FeaturesComputer();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Set the heuristics set to use
        ///
        /// @remark The features computer takes the ownership of the heuristics
        ///         set
        //----------------------------------------------------------------------
        inline void setHeuristicsSet(IHeuristicsSet* pHeuristicsSet)
        {
            assert(!_pHeuristicsSet);
            _pHeuristicsSet = pHeuristicsSet;
        }
        
        //----------------------------------------------------------------------
        /// @brief  Returns the heuristics set used
        //----------------------------------------------------------------------
        inline IHeuristicsSet* heuristicsSet() const
        {
            return _pHeuristicsSet;
        }
        
        //----------------------------------------------------------------------
        /// @brief  Set the seed to use to generate the seeds of the heuristics
        //----------------------------------------------------------------------
        inline void setSeed(unsigned int seed)
        {
            _heuristicsSeed = seed;
        }
        
        //----------------------------------------------------------------------
        /// @brief  Indicates if the Computer has been initialized
        //----------------------------------------------------------------------
        inline bool initialized() const
        {
            return _initialized;
        }

        //----------------------------------------------------------------------
        /// @brief  Add a heuristic in the Computer
        ///
        /// @param  strName     Name of the heuristic
        /// @param  seed        (Optional) Seed of the heuristic
        //----------------------------------------------------------------------
        bool addHeuristic(const std::string& strName, unsigned int seed = 0);

        //----------------------------------------------------------------------
        /// @brief  Initialize the Computer
        ///
        /// @param  nb_views    Number of views
        /// @param  roi_extent  Extent of the region of interest
        //----------------------------------------------------------------------
        bool init(unsigned int nb_views, unsigned int roi_extent);

        //----------------------------------------------------------------------
        /// @brief  Returns the number of heuristics in the Computer
        //----------------------------------------------------------------------
        unsigned int nbHeuristics();

        //----------------------------------------------------------------------
        /// @brief  Returns the number of features (dimension) of a heuristic in
        ///         the Computer
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The dimension of the heuristic, 0 if the index
        ///                     isn't valid
        //----------------------------------------------------------------------
        unsigned int nbFeatures(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Returns the total number of features in the Computer
        //----------------------------------------------------------------------
        unsigned int nbFeaturesTotal();

        //----------------------------------------------------------------------
        /// @brief  Computes several features of the specified heuristic on one
        ///         sample
        ///
        /// @param  sequence    Index of the sequence
        /// @param  image_index Index of the image
        /// @param  pImage      The image
        /// @param  coords      Center of the region of interest
        /// @param  heuristic   Index of the heuristic
        /// @param  nbFeatures  Number of features to compute
        /// @param  indexes     Indexes of the features to compute
        /// @param  values[out] The computed features
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        bool computeSomeFeatures(unsigned int sequence, unsigned int image_index,
                                 Image* pImage, const coordinates_t& coords,
                                 unsigned int heuristic, unsigned int nbFeatures,
                                 unsigned int* indexes, scalar_t* values);

        //----------------------------------------------------------------------
        /// @brief  Put all the heuristics back to their post-initialization
        ///         state
        //----------------------------------------------------------------------
        bool endOfSequence();

        //----------------------------------------------------------------------
        /// @brief  Returns the seed of a heuristic of the set
        ///
        /// @param  heuristic   Index of the heuristic
        /// @return             The seed of the heuristic
        //----------------------------------------------------------------------
        unsigned int heuristicSeed(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Returns the last error that occured
        //----------------------------------------------------------------------
        inline tError getLastError()
        {
            return _pHeuristicsSet->getLastError();
        }


        //_____ Implementation of ImagesCache::IListener __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called when an image is removed from the cache
        //----------------------------------------------------------------------
        virtual void onImageRemoved(unsigned int index);


        //_____ Internal types __________
    protected:
        //----------------------------------------------------------------------
        /// @brief  Definition of a region-of-interest
        //----------------------------------------------------------------------
        struct roi_t
        {
            unsigned int x;
            unsigned int y;
            unsigned int extent;
        };

        //----------------------------------------------------------------------
        /// @brief  Contains all the details about a heuristic
        //----------------------------------------------------------------------
        struct tHeuristicInfos
        {
            unsigned int    index;              ///< Index of the heuristic
            unsigned int    dim;                ///< Dimension of the heuristic (cached for optimization)
            unsigned int    currentSequence;    ///< Sequence currently processed
            unsigned int    currentView;        ///< View currently processed
            unsigned int    currentImage;       ///< Image currently processed
            roi_t           currentROI;         ///< Region-of-interest currently processed
            unsigned int    seed;               ///< Seed of the heuristic
        };
        
        typedef std::vector<tHeuristicInfos>            tHeuristicsList;
        typedef tHeuristicsList::iterator               tHeuristicsIterator;
        typedef std::map<unsigned int, unsigned int>    tEvaluationFeaturesList;
        

        //_____ Attributes __________
    protected:
        IHeuristicsSet* _pHeuristicsSet;    ///< The heuristics set
        tHeuristicsList _heuristics;        ///< Additional infos about the heuristics in the set
        bool            _initialized;       ///< Indicates if the Computer has been initialized
        unsigned int    _heuristicsSeed;    ///< The seed used to generate the seeds of the heuristics
        unsigned int    _nbFeaturesTotal;   ///< Total number of features in the Computer
    };
}

#endif
