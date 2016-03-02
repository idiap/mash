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


/** @file   image_database.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ImageDatabase' class
*/

#ifndef _MASH_IMAGEDATABASE_H_
#define _MASH_IMAGEDATABASE_H_

#include "declarations.h"
#include <mash/images_cache.h>
#include <mash-network/client.h>
#include <mash-utils/arguments_list.h>
#include <vector>
#include <assert.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Represents a database of images containing some labelled objects
    //--------------------------------------------------------------------------
    class MASH_SYMBOL ImageDatabase
    {
        //_____ Internal types __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Contains some informations about an object in an image
        //----------------------------------------------------------------------
        struct tObject
        {
            unsigned int    label;          ///< Label of the object
            coordinates_t   top_left;       ///< Top-left corner of the object
            coordinates_t   bottom_right;   ///< Bottom-right corner of the object
            float           scale;          ///< The scale of the target image
        };

        typedef std::vector<tObject>        tObjectsList;
        typedef tObjectsList::iterator      tObjectsIterator;

        //----------------------------------------------------------------------
        /// @brief  Enumerates the image sets
        //----------------------------------------------------------------------
        enum tImageSet
        {
            SET_NONE,
            SET_TRAINING,
            SET_TEST,
        };
        
        //----------------------------------------------------------------------
        /// @brief  Contains some informations about an image
        //----------------------------------------------------------------------
        struct tImage
        {
            dim_t           size;           ///< Dimensions of the image
            tObjectsList    objects;        ///< List of objects in the image
            tImageSet       set;            ///< Set of the image
        };

        typedef std::vector<tImage>         tImagesList;
        typedef tImagesList::iterator       tImagesIterator;
        
        //----------------------------------------------------------------------
        /// @brief  Contains some informations about a label
        //----------------------------------------------------------------------
        struct tLabel
        {
            std::string     strName;        ///< Name of the label
            unsigned int    nbObjects;      ///< Number of objects with that label in the database
        };

        typedef std::vector<tLabel>         tLabelsList;
        typedef tLabelsList::iterator       tLabelsIterator;


        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// param   maxNbImagesInCache      Maximum number of images stored in
        ///                                 the cache
        //----------------------------------------------------------------------
        ImageDatabase(unsigned int maxNbImagesInCache);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~ImageDatabase();


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Set the client object to use to communicate with the
        ///         application server
        ///
        /// @param  pClient         Client object to use. Must be already
        ///                         connected to an application server.
        /// @return                 Error code
        ///
        /// @remark Takes ownership of the client object
        //----------------------------------------------------------------------
        tError setClient(Client* pClient);

        //----------------------------------------------------------------------
        /// @brief  Retrieves the client object used
        ///
        /// @return The client object used
        //----------------------------------------------------------------------
        inline Client* getClient()
        {
            return _pClient;
        }

        //----------------------------------------------------------------------
        /// @brief  Set the database to use
        ///
        /// @param  strName                     Name of the database
        /// @param  enabledLabels               List of the enabled labels,
        ///                                     empty for all
        /// @param  bBackgroundImagesEnabled    Indicates if the background
        ///                                     images are enabled
        /// @return                             Error code
        //----------------------------------------------------------------------
        tError setDatabase(const std::string& strName,
                           const ArgumentsList& enabledLabels,
                           bool bBackgroundImagesEnabled);

        //----------------------------------------------------------------------
        /// @brief  Returns the preferred image size of the database
        //----------------------------------------------------------------------
        inline dim_t preferredImageSize() const
        {
            return _preferredImageSize;
        }
        
        //----------------------------------------------------------------------
        /// @brief  Returns the preferred ROI size of the database
        //----------------------------------------------------------------------
        inline unsigned int preferredRoiSize() const
        {
            return _preferredRoiSize;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of images in the database
        //----------------------------------------------------------------------
        inline unsigned int nbImages() const
        {
            return _images.size();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the size of the specified image
        ///
        /// @param  image   Index of the image
        /// @return         The size
        //----------------------------------------------------------------------
        inline dim_t imageSize(unsigned int image)
        {
            assert(image < nbImages());

            return _images[image].size;
            
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the set of the specified image
        ///
        /// @param  image   Index of the image
        /// @return         The set
        //----------------------------------------------------------------------
        inline tImageSet imageSet(unsigned int image)
        {
            assert(image < nbImages());

            return _images[image].set;
            
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the list of objects in the specified image
        ///
        /// @param  image   Index of the image
        /// @return         The list of objects (don't delete it!)
        //----------------------------------------------------------------------
        inline tObjectsList* objectsOfImage(unsigned int image)
        {
            assert(image < nbImages());

            return &_images[image].objects;
            
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of objects in the database
        //----------------------------------------------------------------------
        inline unsigned int nbObjects() const
        {
            return _nbObjects;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of objects in the database
        //----------------------------------------------------------------------
        inline unsigned int nbObjects(unsigned int label) const
        {
            assert(label < nbLabels());

            return _labels[label].nbObjects;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the name of the specified label
        ///
        /// @param  label   The label (from 0 to nbLabels()-1)
        //----------------------------------------------------------------------
        inline std::string labelName(unsigned int label) const
        {
            assert(label < nbLabels());

            return _labels[label].strName;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of labels in the database
        //----------------------------------------------------------------------
        inline unsigned int nbLabels() const
        {
            return _labels.size();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the specified image
        ///
        /// @param  index   Index of the image (from 0 to nbImages()-1)
        //----------------------------------------------------------------------
        Image* getImage(unsigned int index);

        //----------------------------------------------------------------------
        /// @brief  Returns the URL of the specified image
        ///
        /// @param  index   Index of the image (from 0 to nbImages()-1)
        //----------------------------------------------------------------------
        std::string getImageUrl(unsigned int index);

        //----------------------------------------------------------------------
        /// @brief  Returns the name of the specified image
        ///
        /// @param  index   Index of the image (from 0 to nbImages()-1)
        //----------------------------------------------------------------------
        std::string getImageName(unsigned int index);

        //----------------------------------------------------------------------
        /// @brief  Returns a description of the last ERROR_EXPERIMENT_PARAMETER
        ///         error that occured
        //----------------------------------------------------------------------
        inline std::string getLastError()
        {
            std::string ret = _strLastError;
            _strLastError = "";
            return ret;
        }

    private:
        tError receiveInfos(int* nbImages, int* nbLabels);


        //_____ Attributes __________
    private:
        Client*         _pClient;
        dim_t           _preferredImageSize;
        unsigned int    _preferredRoiSize;
        std::string     _strImagesUrlPrefix;
        tImagesList     _images;
        tLabelsList     _labels;
        unsigned int    _nbObjects;
        ImagesCache     _cache;
        std::string     _strLastError;
    };
}

#endif
