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


/** @file   image.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'Image' class
*/

#include "image.h"
#include <memory.h>
#include <assert.h>

using namespace Mash;
    

/************************* CONSTRUCTION / DESTRUCTION *************************/

Image::Image(unsigned int width, unsigned int height, unsigned int view)
: _pixelFormats(0), _width(width), _height(height), _view(view), _rgbBuffer(0),
  _rgbLines(0), _grayBuffer(0), _grayLines(0)
{
    assert(width > 0);
    assert(height > 0);
}


Image::~Image()
{
    delete[] _rgbBuffer;
    delete[] _rgbLines;
    
    delete[] _grayBuffer;
    delete[] _grayLines;
}


/*********************************** METHODS **********************************/

Image* Image::copy() const
{
    Image* pCopy = new Image(_width, _height, _view);

    pCopy->addPixelFormats(_pixelFormats);
    
    if (_rgbBuffer)
        memcpy(pCopy->_rgbBuffer, _rgbBuffer, _width * _height * sizeof(RGBPixel_t));

    if (_grayBuffer)
        memcpy(pCopy->_grayBuffer, _grayBuffer, _width * _height * sizeof(byte_t));

    return pCopy;
}


void Image::addPixelFormats(unsigned int pixelFormats)
{
    _pixelFormats |= pixelFormats;
    
    if ((pixelFormats & PIXELFORMAT_RGB) && !_rgbBuffer)
    {
        _rgbBuffer = new RGBPixel_t[_width * _height];
        _rgbLines = new RGBPixel_t*[_height];

        RGBPixel_t* pLine = _rgbBuffer;
        for (unsigned int y = 0; y < _height; ++y)
        {
            _rgbLines[y] = pLine;
            pLine += _width;
        }
    }
    
    if ((pixelFormats & PIXELFORMAT_GRAY) && !_grayBuffer)
    {
        _grayBuffer = new byte_t[_width * _height];
        _grayLines = new byte_t*[_height];

        byte_t* pLine = _grayBuffer;
        for (unsigned int y = 0; y < _height; ++y)
        {
            _grayLines[y] = pLine;
            pLine += _width;
        }
    }
}
