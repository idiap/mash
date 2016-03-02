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


/** @file   sandbox_perception.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxPerception' class
*/

#ifndef _MASH_SANDBOXPERCEPTION_H_
#define _MASH_SANDBOXPERCEPTION_H_

#include <mash-utils/platform.h>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include "warden.h"
#else
    #include "no_warden.h"
#endif

#include <mash-goalplanning/declarations.h>
#include <mash-goalplanning/perception_interface.h>
#include <mash-sandboxing/communication_channel.h>
#include <mash-utils/outstream.h>


//------------------------------------------------------------------------------
/// @brief  Implementation of the Perception object used by the sandboxed
///         goal-planners to retrieve data from the real Perception object,
///         located in the calling process
//------------------------------------------------------------------------------
class MASH_SYMBOL SandboxPerception: public Mash::IPerception
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    ///
    /// @param  channel         The communication channel to use to communicate
    ///                         with the calling process
    /// @param  pOutStream      The output stream to use (can be 0)
    /// @param  pWardenContext  The warden context to use
    /// @param  bReadOnly       Indicates if the perception is in read-only mode
    //--------------------------------------------------------------------------
    SandboxPerception(const Mash::CommunicationChannel& channel,
                      Mash::OutStream* pOutStream,
                      tWardenContext* pWardenContext,
                      bool bReadOnly = false);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~SandboxPerception();


    //_____ Heuristics-related methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Returns the number of heuristics
    //--------------------------------------------------------------------------
    virtual unsigned int nbHeuristics();

    //--------------------------------------------------------------------------
    /// @brief  Returns the number of features (dimension) of a heuristic
    ///
    /// @param  heuristic   Index of the heuristic
    /// @return             The dimension of the heuristic, 0 if the index
    ///                     isn't valid
    //--------------------------------------------------------------------------
    virtual unsigned int nbFeatures(unsigned int heuristic);

    //--------------------------------------------------------------------------
    /// @brief  Returns the total number of features in the perception
    //--------------------------------------------------------------------------
    virtual unsigned int nbFeaturesTotal();

    //--------------------------------------------------------------------------
    /// @brief  Returns the name of a heuristic
    ///
    /// @param  heuristic   Index of the heuristic
    /// @return             The name of the heuristic, empty if the index
    ///                     isn't valid
    //--------------------------------------------------------------------------
    virtual std::string heuristicName(unsigned int heuristic);

    //--------------------------------------------------------------------------
    /// @brief  Returns the seed of a heuristic
    ///
    /// @param  heuristic   Index of the heuristic
    /// @return             The seed of the heuristic
    //--------------------------------------------------------------------------
    virtual unsigned int heuristicSeed(unsigned int heuristic);

    //--------------------------------------------------------------------------
    /// @brief  Indicates if the given heuristic was referenced by the
    ///         goal-planner model that was loaded
    ///
    /// @param  heuristic   Index of the heuristic
    /// @return             'true' if the heuristic was in the model. When
    ///                     no model is used, 'false'.
    //--------------------------------------------------------------------------
    virtual bool isHeuristicUsedByModel(unsigned int heuristic);

    //--------------------------------------------------------------------------
    /// @brief  Mark a heuristic as used by the classifier model that was
    ///         loaded
    ///
    /// @param  heuristic   Index of the heuristic
    //--------------------------------------------------------------------------
    void markHeuristicAsUsedByModel(unsigned int heuristic)
    {
        return _heuristicsInModel.push_back(heuristic);
    }


    //_____ Views-related methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Returns the number of views
    //--------------------------------------------------------------------------
    virtual unsigned int nbViews();

    //--------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------
    virtual bool computeSomeFeatures(unsigned int view,
                                     const Mash::coordinates_t& coordinates,
                                     unsigned int heuristic,
                                     unsigned int nbFeatures,
                                     unsigned int* indexes,
                                     Mash::scalar_t* values);

    //--------------------------------------------------------------------------
    /// @brief  Returns the dimensions of the specified view
    ///
    /// @param  view    Index of the view
    //--------------------------------------------------------------------------
    virtual Mash::dim_t viewSize(unsigned int view);

    //--------------------------------------------------------------------------
    /// @brief  Returns the extent of the region of interest
    ///
    /// @remark The extent of the region of interest will never change
    ///         during an experiment
    //--------------------------------------------------------------------------
    virtual unsigned int roiExtent();

    //--------------------------------------------------------------------------
    /// @brief  Indicates if this is the beginning of a new sequence
    //--------------------------------------------------------------------------
    virtual bool newSequence() const
    {
        return _newSequence;
    }

    inline void setNewSequence(bool newSequence)
    {
        _newSequence = newSequence;
    }


    //_____ Communication-related methods __________
public:
    bool waitResponse();


     //_____ Attributes __________
 protected:
     Mash::CommunicationChannel _channel;
     Mash::OutStream            _outStream;
     tWardenContext*            _pWardenContext;
     bool                       _bReadOnly;
     std::vector<unsigned int>  _heuristicsInModel;
     std::vector<unsigned int>  _nbFeatures;
     unsigned int               _nbFeaturesTotal;
     std::vector<unsigned int>  _seeds;
     std::vector<Mash::dim_t>   _viewSizes;
     unsigned int               _roiExtent;
     bool						_newSequence;
};

#endif
