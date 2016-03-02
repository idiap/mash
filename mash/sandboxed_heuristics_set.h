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


/** @file   sandboxed_heuristics_set.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxedHeuristicsSet' class
*/

#ifndef _MASH_SANDBOXEDHEURISTICSSET_H_
#define _MASH_SANDBOXEDHEURISTICSSET_H_

#include <mash-utils/declarations.h>
#include <mash-sandboxing/sandbox_controller.h>
#include <mash-sandboxing/declarations.h>
#include "heuristics_set_interface.h"
#include "heuristic.h"


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents a heuristics set held in a sandboxed environment when
    ///         untrusted heuristics can be safely executed
    //--------------------------------------------------------------------------
    class MASH_SYMBOL SandboxedHeuristicsSet: public IHeuristicsSet, ISandboxControllerListener
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        SandboxedHeuristicsSet();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~SandboxedHeuristicsSet();


        //_____ Sandbox management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Create the sandbox
        ///
        /// @param  configuration   Configuration of the sandbox
        /// @return                 Error code
        //----------------------------------------------------------------------
        bool createSandbox(const tSandboxConfiguration& configuration);

        //----------------------------------------------------------------------
        /// @brief  Returns the Sandbox Controller used
        //----------------------------------------------------------------------
        inline SandboxController* sandboxController()
        {
            return &_sandbox;
        }
        
        //----------------------------------------------------------------------
        /// @brief  Returns the context of the last operation
        //----------------------------------------------------------------------
        inline std::string getContext() const
        {
            return _strContext;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the stacktrace of the sandboxed object (used to
        ///         report debugging informations after a crash)
        //----------------------------------------------------------------------
        inline std::string getStackTrace()
        {
            return _sandbox.getStackTrace();
        }


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

        //----------------------------------------------------------------------
        /// @brief  Returns the index of the current heuristic
        //----------------------------------------------------------------------
        inline int currentHeuristic() const
        {
            return _currentHeuristic;
        }


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


        //_____ Additional methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Retrieve the statistics about the usage of a heuristic
        ///
        /// @param  heuristic   Index of the heuristic
        /// @retval statistics  The statistics
        //----------------------------------------------------------------------
        bool reportStatistics(unsigned int heuristic,
                              tHeuristicStatistics* statistics);


        //_____ Implementation of ISandboxControllerListener __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called when an unknown response was received by the sandbox
        ///         controller
        ///
        /// Since it might be the sandboxed object asking for some informations,
        /// this method will be called to handle it.
        ///
        /// @param  message     The response received by the sandbox controller
        /// @return             The result of the processing
        //----------------------------------------------------------------------
        virtual SandboxControllerDeclarations::tCommandProcessingResult
                    processResponse(tSandboxMessage message);


        //_____ Internal types __________
    protected:
        struct tContext
        {
            unsigned int    nb_views;
            unsigned int    roi_extent;
            unsigned int    sequence;
            unsigned int    image_width;
            unsigned int    image_height;
            coordinates_t   coordinates;
        };

        typedef std::vector<tContext>   tContextsList;
        typedef tContextsList::iterator tContextsIterator;


        //_____ Attributes __________
    protected:
        SandboxController   _sandbox;           ///< The sandbox used
        OutStream           _outStream;         ///< Output stream to use for logging
        tContextsList       _contexts;
        int                 _currentHeuristic;
        int                 _last_sent_sequence;
        int                 _last_sent_image_index;
        std::string         _strContext;        ///< Context of the sandboxed object (used to report
                                                ///  debugging informations after a crash)
        tError              _lastError;         ///< Last error that occured
    };
}

#endif
