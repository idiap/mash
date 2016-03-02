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


/** @file   dataset.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'DataSet' class
*/

#ifndef _MASH_DATASET_H_
#define _MASH_DATASET_H_

#include "declarations.h"
#include "image_database.h"
#include "stepper.h"
#include <mash-network/client.h>
#include <mash-utils/random_number_generator.h>
#include <vector>
#include <assert.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents the set of data (images, objects, labels) used by the
    ///         classifiers
    ///
    /// This object is used to generate all the needed data from the content of
    /// an image database, by rescaling it appropriately. Thus, the classifier
    /// can have access to much more images than what is effectively stored in
    /// the database.
    //--------------------------------------------------------------------------
    class MASH_SYMBOL DataSet
    {
        //_____ Internal types __________
    public:
        //----------------------------------------------------------------------
        /// @brief  The available modes of the dataset
        //----------------------------------------------------------------------
        enum tMode
        {
            MODE_NORMAL,    ///< Full access to all the images and objects
            MODE_TRAINING,  ///< Restricted access: training set only
            MODE_TEST,      ///< Restricted access: test set only
        };


        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// param   maxNbImagesInCache  Maximum number of images stored in the
        ///                             cache
        //----------------------------------------------------------------------
        DataSet(unsigned int maxNbImagesInCache);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~DataSet();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Setup the dataset
        ///
        /// param   pDatabase           The image database to use
        /// @param  roi_extent          Extent of the region-of-interest
        /// @param  bUseStandardSets    Use the training sets defined by the
        ///                             database
        /// @param  trainingRatio       Ratio of training samples
        /// @param  seed                Seed to use for the generation of the
        ///                             training and testing sets
        /// @param  pCacheListener      Listener of the images cache
        //----------------------------------------------------------------------
        void setup(ImageDatabase* pDatabase, unsigned int roi_extent,
                   Stepper* stepper, bool bUseStandardSets, float trainingRatio,
                   unsigned int seed, ImagesCache::IListener* pCacheListener = 0);

        //----------------------------------------------------------------------
        /// @brief  Change the mode of the dataset
        ///
        /// param   mode    The new mode
        //----------------------------------------------------------------------
        inline void setMode(tMode mode)
        {
            _mode = mode;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the mode of the dataset
        //----------------------------------------------------------------------
        inline tMode getMode() const
        {
            return _mode;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of images in the database
        //----------------------------------------------------------------------
        inline unsigned int nbImages() const
        {
            if (_mode == MODE_NORMAL)
                return _images.size() + _backgroundImages.size();
            else if (_mode == MODE_TRAINING)
                return _trainingImages.size();
            else
                return _testImages.size();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the list of objects in the specified image
        ///
        /// @param  image   Index of the image
        /// @retval objects The list of objects
        //----------------------------------------------------------------------
        void objectsOfImage(unsigned int image, tObjectsList* objects);

        //----------------------------------------------------------------------
        /// @brief  Returns the specified image
        ///
        /// @param  index   Index of the image (from 0 to nbImages()-1)
        //----------------------------------------------------------------------
        Image* getImage(unsigned int index);

        //----------------------------------------------------------------------
        /// @brief  Returns the size of the specified image
        ///
        /// @param  image   Index of the image
        /// @return         The size
        //----------------------------------------------------------------------
        dim_t imageSize(unsigned int image);

        //----------------------------------------------------------------------
        /// @brief  Returns the extent of the region of interest
        ///
        /// @remark The extent of the region of interest will never change
        ///         during an experiment
        //----------------------------------------------------------------------
        unsigned int roiExtent()
        {
            return _roi_extent;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the 'true index' of an image
        ///
        /// The 'true index' is unique across all modes
        /// @param  image   Index of the image in the current mode (from 0 to
        ///                 nbImages()-1)
        //----------------------------------------------------------------------
        unsigned int getImageIndex(unsigned int image);

        //----------------------------------------------------------------------
        /// @brief  Returns some informations about an image
        ///
        /// @param  image       'True index' of the image
        /// @retval strName     Name of the image
        /// @retval size        Size of the image
        /// @retval scale       Scale of the image
        /// @retval training    Indicates if the image is part of the training
        ///                     set
        /// @retval set_index   Index of the image in its set
        //----------------------------------------------------------------------
        void getImageInfos(unsigned int image, std::string* strName, dim_t* size,
                           scalar_t* scale, bool* training,
                           unsigned int* set_index);

        //----------------------------------------------------------------------
        /// @brief  Indicates if the image is part of the 'test set'
        ///
        /// @param  image   'True index' of the image
        //----------------------------------------------------------------------
        bool isImageInTestSet(unsigned int image);


        //_____ Internal types __________
    private:
        //----------------------------------------------------------------------
        /// @brief  Contains some informations about a generated image
        //----------------------------------------------------------------------
        struct tGeneratedImage
        {
            unsigned int    original_image; ///< Index of the original image (in the database)
            float           scale;          ///< Scale of the generated image
        };

        typedef std::vector<tGeneratedImage>    tGeneratedImagesList;
        typedef tGeneratedImagesList::iterator  tGeneratedImagesIterator;

        typedef std::vector<unsigned int>   tIndicesList;
        typedef tIndicesList::iterator      tIndicesIterator;


        //_____ Attributes __________
    private:
        tMode                   _mode;
        ImageDatabase*          _pDatabase;
        unsigned int            _roi_extent;
        Stepper*                _stepper;
        RandomNumberGenerator   _generator;
        tGeneratedImagesList    _images;
        tIndicesList            _backgroundImages;
        ImagesCache             _cache;
        tIndicesList            _trainingImages;
        tIndicesList            _testImages;
    };
}

#endif
