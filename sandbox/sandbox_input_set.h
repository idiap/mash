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


/** @file   sandbox_input_set.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxInputSet' class
*/

#ifndef _MASH_SANDBOXINPUTSET_H_
#define _MASH_SANDBOXINPUTSET_H_

#include <mash-utils/platform.h>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include "warden.h"
#else
    #include "no_warden.h"
#endif

#include <mash-classification/declarations.h>
#include <mash-classification/classifier_input_set_interface.h>
#include <mash-sandboxing/communication_channel.h>
#include <mash-utils/outstream.h>
#include <vector>


//------------------------------------------------------------------------------
/// @brief  Implementation of the Classifier Input Set used by the sandboxed
///         classifiers to retrieve data from the real Input Set, located in
///         the calling process
//------------------------------------------------------------------------------
class MASH_SYMBOL SandboxInputSet: public Mash::IClassifierInputSet
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
    /// @param  bReadOnly       Indicates if the input set is in read-only mode
    //--------------------------------------------------------------------------
    SandboxInputSet(const Mash::CommunicationChannel& channel,
                    Mash::OutStream* pOutStream,
                    tWardenContext* pWardenContext,
                    bool bReadOnly = false);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~SandboxInputSet();


    //_____ Methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Returns the ID of the Inputs Set
    ///
    /// Each time something change in the Set (like the available images),
    /// the ID is changed too.
    //--------------------------------------------------------------------------
    virtual unsigned int id()
    {
        return _id;
    }

	//--------------------------------------------------------------------------
    /// @brief  Indicates if the task is an object detection one
    //--------------------------------------------------------------------------
    virtual bool isDoingDetection() const
    {
        return _bDoingDetection;
    }

    inline void setup(unsigned int id, bool bDoingDetection)
    {
        _id = id;
        _bDoingDetection = bDoingDetection;
        _imageSizes.clear();
    }


    //_____ Heuristics-related methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Returns the number of heuristics in the set
    //--------------------------------------------------------------------------
    virtual unsigned int nbHeuristics();

    //--------------------------------------------------------------------------
    /// @brief  Returns the number of features (dimension) of a heuristic of
    ///         the set
    ///
    /// @param  heuristic   Index of the heuristic
    /// @return             The dimension of the heuristic, 0 if the index
    ///                     isn't valid
    //--------------------------------------------------------------------------
    virtual unsigned int nbFeatures(unsigned int heuristic);

    //--------------------------------------------------------------------------
    /// @brief  Returns the total number of features of the set
    //--------------------------------------------------------------------------
    virtual unsigned int nbFeaturesTotal();

    //--------------------------------------------------------------------------
    /// @brief  Returns the name of a heuristic of the set
    ///
    /// @param  heuristic   Index of the heuristic
    /// @return             The name of the heuristic, empty if the index
    ///                     isn't valid
    //--------------------------------------------------------------------------
    virtual std::string heuristicName(unsigned int heuristic);

    //--------------------------------------------------------------------------
    /// @brief  Returns the seed of a heuristic of the set
    ///
    /// @param  heuristic   Index of the heuristic
    /// @return             The seed of the heuristic
    //--------------------------------------------------------------------------
    virtual unsigned int heuristicSeed(unsigned int heuristic);

    //--------------------------------------------------------------------------
    /// @brief  Indicates if the given heuristic was referenced by the
    ///         classifier model that was loaded
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


    //_____ Images-related methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Returns the number of images in the set
    //--------------------------------------------------------------------------
    virtual unsigned int nbImages();

    //--------------------------------------------------------------------------
    /// @brief  Returns the number of labels in the set
    //--------------------------------------------------------------------------
    virtual unsigned int nbLabels();

    //--------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------
    virtual bool computeSomeFeatures(unsigned int image,
                                     const Mash::coordinates_t& coordinates,
                                     unsigned int heuristic,
                                     unsigned int nbFeatures,
                                     unsigned int* indexes,
                                     Mash::scalar_t* values);

    //--------------------------------------------------------------------------
    /// @brief  Returns the list of the objects in the specified image
    ///
    /// @param  image   Index of the image
    /// @retval objects The list of objects (with their labels) found in the
    ///                 image
    //--------------------------------------------------------------------------
    virtual void objectsInImage(unsigned int image, Mash::tObjectsList* objects);

    //--------------------------------------------------------------------------
    /// @brief  Returns the list of negatives (positions without any objects)
    ///         in the specified image
    ///
    /// @param  image           Index of the image
    /// @param  positions[out]  The positions of the computed negatives
    //--------------------------------------------------------------------------
    virtual void negativesInImage(unsigned int image,
                                  Mash::tCoordinatesList* positions);

    //--------------------------------------------------------------------------
    /// @brief  Returns the dimensions of the specified image
    ///
    /// @param  image   Index of the image
    ///
    /// @remark For images classification, all the images have the same size
    //--------------------------------------------------------------------------
    virtual Mash::dim_t imageSize(unsigned int image);

    //--------------------------------------------------------------------------
    /// @brief  Indicates if the image is part of the 'test set'
    ///
    /// @param  image   Index of the image
    ///
    /// @remark Only useful for the instruments, which can have access to
    ///         the full list of images
    //--------------------------------------------------------------------------
    virtual bool isImageInTestSet(unsigned int image);

    //--------------------------------------------------------------------------
    /// @brief  Returns the extent of the region of interest
    ///
    /// @remark The extent of the region of interest will never change
    ///         during an experiment
    //--------------------------------------------------------------------------
    virtual unsigned int roiExtent();


    //_____ Communication-related methods __________
public:
    bool waitResponse();


    //_____ Attributes __________
protected:
    Mash::CommunicationChannel      _channel;
    Mash::OutStream                 _outStream;
    tWardenContext*                 _pWardenContext;
    bool                            _bReadOnly;
    unsigned int                    _id;
    bool                            _bDoingDetection;
    std::vector<unsigned int>       _heuristicsInModel;
    std::vector<unsigned int>       _nbFeatures;
    unsigned int                    _nbFeaturesTotal;
    std::vector<unsigned int>       _seeds;
    std::vector<Mash::dim_t>        _imageSizes;
    std::vector<char>               _imageInTestSet;
    unsigned int                    _nbLabels;
    unsigned int                    _roiExtent;
};

#endif
