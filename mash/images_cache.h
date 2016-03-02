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


/** @file   images_cache.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ImagesCache' class
*/

#ifndef _MASH_IMAGESCACHE_H_
#define _MASH_IMAGESCACHE_H_

#include <mash-utils/declarations.h>
#include "image.h"
#include "heuristic.h"
#include <map>
#include <vector>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Used to maintain a cache of images
    //--------------------------------------------------------------------------
    class MASH_SYMBOL ImagesCache
    {
        //_____ Internal types __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Interface to be implemented by the object that want to be
        ///         notified about the events happening in the cache
        //----------------------------------------------------------------------
        class IListener
        {
        public:
            //------------------------------------------------------------------
            /// @brief  Destructor
            //------------------------------------------------------------------
            virtual ~IListener() {}
            
            //------------------------------------------------------------------
            /// @brief  Called when an image is removed from the cache
            //------------------------------------------------------------------
            virtual void onImageRemoved(unsigned int index) = 0;
        };
 
        
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param  size    Maximum number of images that will be stored in the
        ///                 cache
        //----------------------------------------------------------------------
        ImagesCache(unsigned int size);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~ImagesCache();
    

        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Returns the size of the cache
        //----------------------------------------------------------------------
        inline unsigned int size() const
        {
            return _size;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of images in the cache
        //----------------------------------------------------------------------
        inline unsigned int nbImages() const
        {
            return _cached.size;
        }
        
        //----------------------------------------------------------------------
        /// @brief  Set the listener
        //----------------------------------------------------------------------
        inline void setListener(IListener* pListener)
        {
            _pListener = pListener;
        }
 
        
        //----------------------------------------------------------------------
        /// @brief  Add an image in the cache
        ///
        /// @param  index   Index of the image
        /// @param  pImage  The image
        ///
        /// @remark The ownership of the image is transfered to the cache
        //----------------------------------------------------------------------
        void addImage(unsigned int index, Image* pImage);

        //----------------------------------------------------------------------
        /// @brief  Returns one image from the cache
        ///
        /// @param  index   Index of the image
        /// @return         The image, 0 if not in the cache
        //----------------------------------------------------------------------
        Image* getImage(unsigned int index);
        
        //----------------------------------------------------------------------
        /// @brief  Clear the cache
        //----------------------------------------------------------------------
        void clear();


        //_____ Internal types __________
    private:
        //----------------------------------------------------------------------
        /// @brief  Represents an image, held in a double linked list
        //----------------------------------------------------------------------
        struct tImage
        {
            unsigned int    index;      ///< Index of the image
            Image*          pImage;     ///< The image
            tImage*         next;       ///< Next element in the list
            tImage*         previous;   ///< Previous element in the list
        };


        //----------------------------------------------------------------------
        /// @brief  Represents a double linked list of images
        //----------------------------------------------------------------------
        struct tList
        {
            tImage*         head;       ///< First element of the list
            tImage*         tail;       ///< Last element of the list
            unsigned int    size;       ///< Number of elements in the list
        };


        //_____ Attributes __________
    private:
        unsigned int    _size;          ///< Size of the cache (maximum number of images)
        tList           _cached;        ///< List of the cached images
        IListener*      _pListener;     ///< The listener to notify about the events
                                        ///  happening in the cache
    };
}

#endif
