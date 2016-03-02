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


/** @file   sandbox_notifier_proxy.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxNotifierProxy' class
*/

#ifndef _MASH_SANDBOXNOTIFIERPROXY_H_
#define _MASH_SANDBOXNOTIFIERPROXY_H_

#include "notifier_interface.h"
#include <mash-sandboxing/communication_channel.h>
#include <mash-sandboxing/sandbox_controller.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Proxy that process the notifications sent by a sandboxed plugin
    //--------------------------------------------------------------------------
    class MASH_SYMBOL SandboxNotifierProxy
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        SandboxNotifierProxy(INotifier* pNotifier, const CommunicationChannel& channel);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~SandboxNotifierProxy();


        //_____ Command handlers __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Process a message
        ///
        /// @param  message     The message received
        /// @return             The result of the processing
        //----------------------------------------------------------------------
        SandboxControllerDeclarations::tCommandProcessingResult processResponse(tSandboxMessage message);

    protected:
        SandboxControllerDeclarations::tCommandProcessingResult handleTrainingStepDoneNotification();
        

        //_____ Internal types __________
    protected:
        typedef SandboxControllerDeclarations::tCommandProcessingResult (SandboxNotifierProxy::*tNotificationHandler)();

        typedef std::map<tSandboxMessage, tNotificationHandler>  tNotificationHandlersList;
        typedef tNotificationHandlersList::iterator              tNotificationHandlersIterator;

    
        //_____ Attributes __________
    protected:
        static tNotificationHandlersList handlers;

        CommunicationChannel    _channel;
        INotifier*              _pNotifier;
    };
}

#endif
