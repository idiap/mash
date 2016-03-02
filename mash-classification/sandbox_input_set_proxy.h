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


/** @file   sandbox_input_set_proxy.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxInputSetProxy' class
*/

#ifndef _MASH_SANDBOXINPUTSETPROXY_H_
#define _MASH_SANDBOXINPUTSETPROXY_H_

#include "declarations.h"
#include "classifier_input_set_interface.h"
#include <mash-sandboxing/communication_channel.h>
#include <mash-sandboxing/sandbox_controller.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Proxy that process the commands sent by a sandboxed plugin that
    ///         want to use an Input Set
    //--------------------------------------------------------------------------
    class MASH_SYMBOL SandboxInputSetProxy
    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        SandboxInputSetProxy(IClassifierInputSet* pInputSet, const CommunicationChannel& channel);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~SandboxInputSetProxy();


        //_____ Command handlers __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Process a message
        ///
        /// @param  message     The response received
        /// @return             The result of the processing
        //----------------------------------------------------------------------
        SandboxControllerDeclarations::tCommandProcessingResult processResponse(tSandboxMessage message);

    protected:
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetNbHeuristicsCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetNbFeaturesCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetHeuristicNameCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetHeuristicSeedCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetNbImagesCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetNbLabelsCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetComputeSomeFeaturesCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetObjectsInImageCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetNegativesInImageCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetImageSizeCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetImageInTestSetCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleInputSetRoiExtentCommand();
        

        //_____ Internal types __________
    protected:
        typedef SandboxControllerDeclarations::tCommandProcessingResult (SandboxInputSetProxy::*tCommandHandler)();

        typedef std::map<tSandboxMessage, tCommandHandler>  tCommandHandlersList;
        typedef tCommandHandlersList::iterator              tCommandHandlersIterator;

    
        //_____ Attributes __________
    protected:
        static tCommandHandlersList handlers;

        CommunicationChannel    _channel;
        IClassifierInputSet*    _pInputSet;
    };
}

#endif
