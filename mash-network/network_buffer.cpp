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


/**	@file	network_buffer.cpp
	@author	Philip Abbet (philip.abbet@idiap.ch)

	Implementation of the 'NetworkBuffer' class
*/

#include "network_buffer.h"
#include <memory.h>
#include <assert.h>


using namespace Mash;


const unsigned int BUFFER_INCREMENT = 1024;


/****************************** CONSTRUCTION / DESTRUCTION ******************************/

NetworkBuffer::NetworkBuffer()
: _data(0), _size(0), _allocated(BUFFER_INCREMENT)
{
    _data = new unsigned char[_allocated];
}

//-----------------------------------------------------------------------

NetworkBuffer::NetworkBuffer(const std::string& str)
: _data(0), _size(str.size()), _allocated(((str.size() / BUFFER_INCREMENT) + 1) * BUFFER_INCREMENT)
{
    _data = new unsigned char[_allocated];
    memcpy(_data, str.c_str(), _size);
}

//-----------------------------------------------------------------------

NetworkBuffer::NetworkBuffer(const unsigned char* pData, unsigned int dataSize)
: _data(0), _size(dataSize), _allocated(((dataSize / BUFFER_INCREMENT) + 1) * BUFFER_INCREMENT)
{
    assert(pData);
    assert(dataSize > 0);

    _data = new unsigned char[_allocated];
    memcpy(_data, pData, _size);
}

//-----------------------------------------------------------------------

NetworkBuffer::~NetworkBuffer()
{
    delete[] _data;
}


/*********************************** METHODS **********************************/

void NetworkBuffer::add(const std::string& str)
{
    if (_size + str.size() > _allocated)
        reallocate(str.size());
    
    memcpy(_data + _size, str.c_str(), str.size());
    
    _size += str.size();
}


void NetworkBuffer::add(const unsigned char* pData, unsigned int dataSize)
{
    assert(pData);
    assert(dataSize > 0);

    if (_size + dataSize > _allocated)
        reallocate(dataSize);
    
    memcpy(_data + _size, pData, dataSize);
    
    _size += dataSize;
}


void NetworkBuffer::extract(unsigned char* pDest, unsigned int nbBytes)
{
    assert(pDest);
    assert(nbBytes <= _size);
    
    memcpy(pDest, _data, nbBytes);
    
    unsigned char* previous = _data;

    _size -= nbBytes;

    _data = new unsigned char[_allocated];
    memcpy(_data, previous + nbBytes, _size);
    
    delete[] previous;
}


bool NetworkBuffer::extractLine(std::string &strLine)
{
    for (unsigned int i = 0; i < _size; ++i)
    {
        if (_data[i] == '\n')
        {
            _data[i] = 0;
            strLine = (char*) _data;
            
            unsigned char* previous = _data;

            _size -= i + 1;

            _data = new unsigned char[_allocated];
            memcpy(_data, previous + i + 1, _size);

            delete[] previous;

            return true;
        }
    }
    
    return false;
}


void NetworkBuffer::reset()
{
    _size = 0;
}


void NetworkBuffer::reallocate(unsigned int nbBytesToAdd)
{
    if (nbBytesToAdd <= _allocated - _size)
        return;

    _allocated = (((_size + nbBytesToAdd) / BUFFER_INCREMENT) + 1) * BUFFER_INCREMENT;
    
    unsigned char* previous = _data;

    _data = new unsigned char[_allocated];
    memcpy(_data, previous, _size);
    
    delete[] previous;
}
