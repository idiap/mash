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


/** @file   sandbox_controller_listener.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ISandboxControllerListener' interface
*/

#ifndef _MASH_ISANDBOXCONTROLLERLISTENER_H_
#define _MASH_ISANDBOXCONTROLLERLISTENER_H_

#include "declarations.h"
#include "sandbox_messages.h"


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Implementation of the interface of the object used by the Input
    ///         Set to report events
    //--------------------------------------------------------------------------
    class MASH_SYMBOL ISandboxControllerListener
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        ISandboxControllerListener()
        {
        }
        
        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        virtual ~ISandboxControllerListener()
        {
        }


        //_____ Events __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Called when an unknown response was received by the sandbox
        ///         controller
        ///
        /// Since it might be the sandboxed object asking for some informations,
        /// this method will be called to handle it.
        ///
        /// @param  message     The response received by the sandbox controller
        /// @return             The result of the processing
        //----------------------------------------------------------------------
        virtual SandboxControllerDeclarations::tCommandProcessingResult
                    processResponse(tSandboxMessage message)
        {
            return SandboxControllerDeclarations::COMMAND_UNKNOWN;
        }
    };
}

#endif
