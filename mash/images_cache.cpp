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


/** @file   images_cache.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'ImagesCache' class
*/

#include "images_cache.h"
#include <assert.h>

using namespace Mash;
    

/************************* CONSTRUCTION / DESTRUCTION *************************/

ImagesCache::ImagesCache(unsigned int size)
: _size(size), _pListener(0)
{
    assert(size > 0);
    
    _cached.head = 0;
    _cached.tail = 0;
    _cached.size = 0;
}


ImagesCache::~ImagesCache()
{
    clear();
}


/*********************************** METHODS **********************************/

void ImagesCache::addImage(unsigned int index, Image* pImage)
{
    // Assertions
    assert(pImage);
    
    // Test if the older image must be removed
    if (_cached.size >= _size)
    {
        assert(_cached.tail);
        
        tImage* pLast = _cached.tail;
        
        if (pLast->previous)
            pLast->previous->next = 0;
        
        _cached.tail = pLast->previous;
        if (!_cached.tail)
            _cached.head = 0;

        if (_pListener)
            _pListener->onImageRemoved(pLast->index);

        delete pLast->pImage;
        delete pLast;

        --_cached.size;
    }
    
    // Add a new element at the beginning of the list
    tImage* pCached     = new tImage();
    pCached->index      = index;
    pCached->pImage     = pImage;
    pCached->next       = _cached.head;
    pCached->previous   = 0;
    
    if (_cached.head)
        _cached.head->previous = pCached;
    
    _cached.head = pCached;
    
    if (!_cached.tail)
        _cached.tail = _cached.head;
    
    ++_cached.size;
}


Image* ImagesCache::getImage(unsigned int index)
{
    // Search the element in the list holding that image
    tImage* pCurrent = _cached.head;
    while (pCurrent)
    {
        if (pCurrent->index == index)
        {
            // Move it to the beginning of the list
            if (pCurrent != _cached.head)
            {
                if (pCurrent == _cached.tail)
                {
                    pCurrent->previous->next = 0;
                    _cached.tail = pCurrent->previous;
                }
                else
                {
                    pCurrent->previous->next = pCurrent->next;
                    pCurrent->next->previous = pCurrent->previous;
                }

                pCurrent->next = _cached.head;
                pCurrent->previous = 0;

                _cached.head->previous = pCurrent;
                _cached.head = pCurrent;
            }
            
            return pCurrent->pImage;
        }
                    
        pCurrent = pCurrent->next;
    }

    return 0;
}


void ImagesCache::clear()
{
    tImage* pCurrent = _cached.head;
    while (pCurrent)
    {
        tImage* pNext = pCurrent->next;
        
        if (_pListener)
            _pListener->onImageRemoved(pCurrent->index);
        
        delete pCurrent->pImage;
        delete pCurrent;
        pCurrent = pNext;
    }
    
    _cached.head = 0;
    _cached.tail = 0;
    _cached.size = 0;
}
