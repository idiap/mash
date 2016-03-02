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


/** @file   busy_listener.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'BusyListener' class
*/

#include "busy_listener.h"
#include "networkutils.h"
#include <assert.h>

using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

BusyListener::BusyListener(int socket, tServerListenerConstructor* stdListenerConstructor)
: ServerListener(socket), _stdListenerConstructor(stdListenerConstructor)
{
    // Assertions
    assert(stdListenerConstructor);
}


BusyListener::~BusyListener()
{
}


/********************** IMPLEMENTATION OF ServerListener **********************/

ServerListener::tAction BusyListener::handleCommand(const std::string& strCommand,
                                                    const ArgumentsList& arguments)
{
    // Assertions
    assert(_stdListenerConstructor);

    if (strCommand == "INFO")
    {
        ServerListener* pListener = _stdListenerConstructor(_socket);
        tAction action = pListener->handleCommand(strCommand, arguments);
        delete pListener;
        return action;
    }
    else if (strCommand == "DONE")
    {
        sendResponse("GOODBYE", ArgumentsList());
        return ACTION_CLOSE_CONNECTION;
    }
    else if (strCommand == "SLEEP")
    {
        sendResponse("OK", ArgumentsList());
        return ACTION_SLEEP;
    }
    else
    {
        sendResponse("BUSY", ArgumentsList());
        return ACTION_CLOSE_CONNECTION;
    }
}
