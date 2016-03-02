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


/** @file   trusted_heuristics_set.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'TrustedHeuristicsSet' class
*/

#ifndef _MASH_TRUSTEDHEURISTICSSET_H_
#define _MASH_TRUSTEDHEURISTICSSET_H_

#include <mash-utils/declarations.h>
#include <mash-utils/outstream.h>
#include "heuristics_set_interface.h"
#include "heuristics_manager.h"
#include "heuristic.h"


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents a set of trusted heuristics
    //--------------------------------------------------------------------------
    class MASH_SYMBOL TrustedHeuristicsSet: public IHeuristicsSet
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        TrustedHeuristicsSet();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~TrustedHeuristicsSet();


        //_____ Management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Configure the set
        ///
        /// @param  strLogFolder    Folder where the log files must be written
        //----------------------------------------------------------------------
        void configure(const std::string& strLogFolder);


        //_____ Heuristics management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Tell the set about the folder into which the heuristic
        ///         plugins are located
        ///
        /// @param  strPath     Path of the folder containing the plugins
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        virtual bool setHeuristicsFolder(const std::string& strPath);

        //----------------------------------------------------------------------
        /// @brief  Tell the set to load a specific heuristic plugin
        ///
        /// @param  strName     Name of the plugin
        /// @return             The index of the plugin, -1 in case of error
        //----------------------------------------------------------------------
        virtual int loadHeuristicPlugin(const std::string& strName);

        //----------------------------------------------------------------------
        /// @brief  Tell the set to create instances of the heuristics defined
        ///         in the loaded plugins
        ///
        /// @return 'true' if successful
        //----------------------------------------------------------------------
        virtual bool createHeuristics();

        //----------------------------------------------------------------------
        /// @brief  Returns the number of heuristics
        //----------------------------------------------------------------------
        virtual unsigned int nbHeuristics() const;

        //----------------------------------------------------------------------
        /// @brief  Returns the index of an heuristic
        ///
        /// @param  strName     Name of the heuristic
        /// @return             The index of the heuristic, -1 if not found
        //----------------------------------------------------------------------
        virtual int heuristicIndex(const std::string& strName) const;

        //----------------------------------------------------------------------
        /// @brief  Returns the name of an heuristic
        ///
        /// @param  index   Index of the heuristic
        /// @return         The name of the heuristic
        //----------------------------------------------------------------------
        virtual std::string heuristicName(int index) const;


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Sets the initial seed that must be used by a heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @param  seed        The initial seed
        //----------------------------------------------------------------------
        virtual bool setSeed(unsigned int heuristic, unsigned int seed);
        
        //----------------------------------------------------------------------
        /// @brief  Called once at the creation of a heuristic
        ///
        /// Pre-computes all the data that will never change during the
        /// life-time of the heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @param  nb_views    The number of views that will be provided to the
        ///                     heuristic 
        /// @param  roi_extent  The extent of the region-of-interest
        //----------------------------------------------------------------------
        virtual bool init(unsigned int heuristic, unsigned int nb_views,
                          unsigned int roi_extent);

        //----------------------------------------------------------------------
        /// @brief  Returns the number of features a heuristic computes
        ///
        /// @param  heuristic   Index of the heuristic
        //----------------------------------------------------------------------
        virtual unsigned int dim(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Called once per sequence, before any computation 
        ///
        /// Allocates and initializes the data a heuristic will need to
        /// compute features on a stream of images (like a video)
        ///
        /// @param  heuristic   Index of the heuristic
        //----------------------------------------------------------------------
        virtual bool prepareForSequence(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Called once per sequence, after any computation 
        ///
        /// Frees the structures allocated by the prepareForSequence() method
        ///
        /// @param  heuristic   Index of the heuristic
        //----------------------------------------------------------------------
        virtual bool finishForSequence(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Called once per image, before any computation 
        ///
        /// Pre-computes from a full image the data a heuristic will need to
        /// compute features at any coordinate in the image
        ///
        /// @param  heuristic   Index of the heuristic
        /// @param  sequence    Index of the sequence
        /// @param  image_index Index of the image in the sequence
        /// @param  image       The image that the heuristic will process. If 0,
        ///                     no image is copied in the process of the
        ///                     heuristic. It is assumed that one was already
        ///                     provided.
        //----------------------------------------------------------------------
        virtual bool prepareForImage(unsigned int heuristic, unsigned int sequence,
                                     unsigned int image_index, const Image* image);

        //----------------------------------------------------------------------
        /// @brief  Called once per image, after any computation 
        ///
        /// Frees the structures allocated by the prepareForImage() method
        ///
        /// @param  heuristic   Index of the heuristic
        //----------------------------------------------------------------------
        virtual bool finishForImage(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Called once per coordinates, before any computation
        ///
        /// Pre-computes the data a heuristic will need to compute features
        /// at the given coordinates
        ///
        /// @param  heuristic   Index of the heuristic
        /// @param  coordinates The coordinates
        //----------------------------------------------------------------------
        virtual bool prepareForCoordinates(unsigned int heuristic,
                                           const coordinates_t& coordinates);

        //----------------------------------------------------------------------
        /// @brief  Called once per coordinates, after any computation 
        ///
        /// Frees the memory allocated by the prepareForCoordinates() method
        ///
        /// @param  heuristic   Index of the heuristic
        //----------------------------------------------------------------------
        virtual bool finishForCoordinates(unsigned int heuristic);

        //----------------------------------------------------------------------
        /// @brief  Computes several features in the current region of interest
        ///
        /// @param  heuristic   Index of the heuristic
        /// @param  nbFeatures  Number of features to compute
        /// @param  indexes     Indexes of the features to compute
        /// @param  values[out] The computed features
        //----------------------------------------------------------------------
        virtual bool computeSomeFeatures(unsigned int heuristic,
                                         unsigned int nbFeatures,
                                         unsigned int* indexes,
                                         scalar_t* values);
    
        //----------------------------------------------------------------------
        /// @brief  Returns the last error that occured
        //----------------------------------------------------------------------
        virtual tError getLastError();


        //_____ Internal types __________
    protected:
        struct tHeuristicInfos
        {
            Mash::Heuristic*    pHeuristic;
            std::string         strName;
            unsigned int        currentSeed;
        };

        typedef std::vector<tHeuristicInfos>    tHeuristicsList;
        typedef tHeuristicsList::iterator       tHeuristicsIterator;
        typedef tHeuristicsList::const_iterator tHeuristicsConstIterator;


        //_____ Attributes __________
    protected:
        HeuristicsManager*  _pManager;      ///< The heuristics manager used
        tHeuristicsList     _heuristics;    ///< The heuristics
        OutStream           _outStream;     ///< Output stream to use for logging
        tError              _lastError;     ///< Last error that occured
    };
}

#endif
