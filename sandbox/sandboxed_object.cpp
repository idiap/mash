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


/** @file   sandboxed_object.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxedObject' class
*/

#include "sandboxed_object.h"


using namespace std;
using namespace Mash;


/****************************** STATIC ATTRIBUTES *****************************/

ISandboxedObject* ISandboxedObject::pInstance = 0;


/************************* CONSTRUCTION / DESTRUCTION *************************/

ISandboxedObject::ISandboxedObject(const CommunicationChannel& channel,
                                   OutStream* pOutStream)
: _channel(channel)
{
    if (pOutStream)
        _outStream = *pOutStream;

    _outStream.setVerbosityLevel(3);

    pInstance = this;

    setWardenListener(warden_listener);
}


ISandboxedObject::~ISandboxedObject()
{
    pInstance = 0;
}


/**************************** WARDEN-RELATED METHODS **************************/

#if MASH_PLATFORM == MASH_PLATFORM_LINUX

void ISandboxedObject::warden_listener(tWardenContext* pContext, tWardenStatus status,
                                       const char* details)
{
    assert(status != WARDEN_STATUS_NONE);
    assert(pInstance);

    setWardenContext(0);

    if (status == WARDEN_STATUS_MEMORY_ALLOCATION_LIMIT)
    {
        pInstance->_outStream << "< MEMORY_LIMIT_REACHED" << endl;

        pInstance->_channel.startPacket(SANDBOX_EVENT_MEMORY_LIMIT_REACHED);
        pInstance->_channel.sendPacket();
    }
    else if (status == WARDEN_STATUS_FORBIDDEN_SYSTEM_CALL)
    {
        pInstance->_outStream << "< FORBIDDEN_SYSTEM_CALL " << details << endl;

        pInstance->_channel.startPacket(SANDBOX_EVENT_FORBIDDEN_SYSTEM_CALL);
        pInstance->_channel.add(details);
        pInstance->_channel.sendPacket();
    }
    else
    {
        pInstance->_outStream << "< ERROR " << details << endl;

        pInstance->_channel.startPacket(SANDBOX_MESSAGE_ERROR);
        pInstance->_channel.add(ERROR_WARDEN);
        pInstance->_channel.add(details);
        pInstance->_channel.sendPacket();
    }

    setWardenContext(pContext);
}

#endif
