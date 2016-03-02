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


/** @file	network_buffer.h
	@author Philip Abbet (philip.abbet@idiap.ch)
    
	Declaration of the class 'NetworkBuffer'
*/

#ifndef _MASH_NETWORKBUFFER_H_
#define _MASH_NETWORKBUFFER_H_

#include <string>


namespace Mash
{
	//--------------------------------------------------------------------------
	/// @brief	Network-related utility class
	//--------------------------------------------------------------------------
	class NetworkBuffer
	{
        friend class NetworkUtils;
	    
	    //_____ Constructor / Destructor __________
	public:
        NetworkBuffer();
        NetworkBuffer(const std::string& str);
        NetworkBuffer(const unsigned char* pData, unsigned int dataSize);
        ~NetworkBuffer();


        //_____ Methods __________
    public:
        void add(const std::string& str);
        void add(const unsigned char* pData, unsigned int dataSize);
    
        void extract(unsigned char* pDest, unsigned int nbBytes);
        bool extractLine(std::string &strLine);
    
        void reset();
    
        inline unsigned int size() const
        {
            return _size;
        }
    
    private:
        void reallocate(unsigned int nbBytesToAdd);
        

        //_____ Attributes __________
    private:
        unsigned char*  _data;
        unsigned int    _size;
        unsigned int    _allocated;
	};
}

#endif