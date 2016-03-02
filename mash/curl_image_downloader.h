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


/** @file   curl_image_downloader.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ImageUtils' class
*/

#ifndef _MASH_CURLIMAGEDOWNLOADER_H_
#define _MASH_CURLIMAGEDOWNLOADER_H_

#ifdef USE_CURL

#include "imageutils.h"
#include <memory.h>
#include <FreeImage.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Base class for an 'Image Downloader' object
    //--------------------------------------------------------------------------
    class MASH_SYMBOL CURLImageDownloader: public IImageDownloader
    {
        /// Used by the download process
        struct MemoryStruct
        {
            char*   memory;
            size_t  size;
        };

        static size_t WriteMemoryCallback(void* ptr, size_t size, size_t nmemb, void* data)
        {
            size_t realsize = size * nmemb;
            struct MemoryStruct* mem = (struct MemoryStruct*) data;

            if (mem->memory)
                mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
            else
                mem->memory = (char*) malloc(mem->size + realsize + 1);

            if (mem->memory)
            {
                memcpy(&(mem->memory[mem->size]), ptr, realsize);
                mem->size += realsize;
                mem->memory[mem->size] = 0;
            }

            return realsize;
        }


    public:
        CURLImageDownloader()
        {
            curl_global_init(CURL_GLOBAL_ALL);
        }
        
        virtual ~CURLImageDownloader()
        {
        }
        
        virtual void* loadImage(const std::string& strUrl)
        {
            FIBITMAP* pBitmap = 0;
            struct MemoryStruct chunk;
            chunk.memory    = 0;
            chunk.size      = 0; 

            // Init the curl session
            CURL* curl_handle = curl_easy_init();

            // Specify URL to get
            curl_easy_setopt(curl_handle, CURLOPT_URL, strUrl.c_str());

            // Send all data to out callback function 
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

            // Pass our 'chunk' structure to the callback function
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*) &chunk);

            // Some servers don't like requests that are made without a user-agent
            // field, so we provide one
            curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

            // Download the file
            CURLcode ret = curl_easy_perform(curl_handle);

            // Cleanup curl stuff
            curl_easy_cleanup(curl_handle);

            if (ret == 0)
            {
                FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(strUrl.c_str());

                // Wrap the downloaded buffer in a FreeImage memory stream
                FIMEMORY* pMemory = FreeImage_OpenMemory((BYTE*) chunk.memory, chunk.size);
                if (pMemory)
                {
                    // Decode the image from the memory stream
                    pBitmap = FreeImage_LoadFromMemory(format, pMemory, 0);

                    FreeImage_CloseMemory(pMemory);
                }
            }

            free(chunk.memory);
            
            return (void*) pBitmap;
        }
    };
}

#endif

#endif
