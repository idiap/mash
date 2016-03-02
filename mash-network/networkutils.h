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


/** @file	networkutils.h
	@author Philip Abbet (philip.abbet@idiap.ch)
    
	Declaration of the class 'NetworkUtils'
*/

#ifndef _MASH_NETWORKUTILS_H_
#define _MASH_NETWORKUTILS_H_

#include <mash-utils/arguments_list.h>
#include "network_buffer.h"
#include <string>
#include <sys/socket.h>


namespace Mash
{
	//--------------------------------------------------------------------------
	/// @brief	Network-related utility class
	//--------------------------------------------------------------------------
	class NetworkUtils
	{
	public:
        static void* getNetworkAddress(struct sockaddr* sa);
        
        static bool sendMessage(int socket, const std::string& strMessage,
                                const ArgumentsList& arguments);

        static bool sendData(int socket, const unsigned char* data, int size);

        static bool waitMessage(int socket, NetworkBuffer* pBuffer,
                                std::string* strMessage, ArgumentsList* arguments,
                                struct timeval* pTimeout = 0);

        static bool waitData(int socket, NetworkBuffer* pBuffer, unsigned char* data, int size);
    
        static bool processBuffer(NetworkBuffer* pBuffer, std::string* strMessage,
                                  ArgumentsList* arguments);

    private:
        static std::string encodeArgument(const std::string& strArgument);
        static std::string decodeArgument(const std::string& strArgument);
	};
}

#endif
