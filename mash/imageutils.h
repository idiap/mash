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


/** @file   imageutils.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ImageUtils' class
*/

#ifndef _MASH_IMAGEUTILS_H_
#define _MASH_IMAGEUTILS_H_

#include "image.h"
#include <string>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Base class for an 'Image Downloader' object
    //--------------------------------------------------------------------------
    class MASH_SYMBOL IImageDownloader
    {
    public:
        virtual ~IImageDownloader() {};
        virtual void* loadImage(const std::string& strUrl) = 0;
    };


    //--------------------------------------------------------------------------
    /// @brief  Utility methods to load, save, convert and manipulate images
    ///
    /// All the methods of this class are static
    //--------------------------------------------------------------------------
    class MASH_SYMBOL ImageUtils
    {
        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Load an image file
        ///
        /// @param  strUrl      URL of the image (or path to a file)
        /// @return             An image, 0 if failed
        //----------------------------------------------------------------------
        static Image* loadImage(const std::string& strUrl);

        //----------------------------------------------------------------------
        /// @brief  Create an image object around a memory buffer
        ///
        /// @param  strMimeType     MIME type of the content of the buffer.
        ///                         Possible values: 'image/ppm', 'image/jpeg',
        ///                         'image/png', 'image/mif' (MASH Image Format)
        /// @param  pBuffer         The memory buffer
        /// @param  size            Size of the memory buffer, in bytes
        /// @return                 An image, 0 if failed
        //----------------------------------------------------------------------
        static Image* createImage(const std::string& strMimeType,
                                  unsigned char* pBuffer, long size);

        //----------------------------------------------------------------------
        /// @brief  Convert an image to some others pixel formats
        ///
        /// @param  pImage          The image
        /// @param  pixelFormats    The pixel formats
        /// @return                 'false' if at least one pixel format wasn't
        ///                         added to the image 
        ///
        /// @remark Only the missing pixel formats are added
        //----------------------------------------------------------------------
        static bool convertImageToPixelFormats(Image* pImage,
                                               unsigned int pixelFormats);

        //----------------------------------------------------------------------
        /// @brief  Rescales an image
        ///
        /// @param  pImage  The image
        /// @param  scale   The scale
        /// @return         The rescaled image
        //----------------------------------------------------------------------
        static Image* scale(Image* pImage, float scale);

        //----------------------------------------------------------------------
        /// @brief  Rescales an image to a fixed size, by padding missing lines
        ///         or rows with a fixed color
        ///
        /// @param  pImage          The image
        /// @param  width           Width of the image
        /// @param  height          Height of the image
        /// @param  paddingColor    Color to use for padding
        /// @return                 The rescaled image
        //----------------------------------------------------------------------
        static Image* scale(Image* pImage, unsigned int width, unsigned int height,
                            RGBPixel_t paddingColor);
        
        
        static void setDownloader(IImageDownloader* pDownloader);


        //_____ Attributes __________
    private:
        static bool                 bInitialised;
        static IImageDownloader*    pDownloader;
    };
}

#endif
