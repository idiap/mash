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


/** @file   sandbox_task_proxy.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxTaskProxy' class
*/

#ifndef _MASH_SANDBOXTASKPROXY_H_
#define _MASH_SANDBOXTASKPROXY_H_

#include "declarations.h"
#include "task.h"
#include <mash-sandboxing/communication_channel.h>
#include <mash-sandboxing/sandbox_controller.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Proxy that process the commands sent by a sandboxed plugin that
    ///         want to use a Task object
    //--------------------------------------------------------------------------
    class MASH_SYMBOL SandboxTaskProxy

    {
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        SandboxTaskProxy(ITask* pTask, const CommunicationChannel& channel);

        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        SandboxTaskProxy(IPerception* pPerception, const CommunicationChannel& channel);

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~SandboxTaskProxy();


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
        void setupHandlers();
        
        SandboxControllerDeclarations::tCommandProcessingResult handleTaskModeCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleTaskNbActionsCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleTaskNbTrajectoriesCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleTaskTrajectoryLengthCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleTaskResetCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleTaskPerformActionCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handleTaskSuggestedActionCommand();

        SandboxControllerDeclarations::tCommandProcessingResult handlePerceptionNbHeuristicsCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handlePerceptionNbFeaturesCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handlePerceptionHeuristicNameCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handlePerceptionHeuristicSeedCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handlePerceptionNbViewsCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handlePerceptionComputeSomeFeaturesCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handlePerceptionViewSizeCommand();
        SandboxControllerDeclarations::tCommandProcessingResult handlePerceptionRoiExtentCommand();
        

        //_____ Internal types __________
    protected:
        typedef SandboxControllerDeclarations::tCommandProcessingResult (SandboxTaskProxy::*tCommandHandler)();

        typedef std::map<tSandboxMessage, tCommandHandler>  tCommandHandlersList;
        typedef tCommandHandlersList::iterator              tCommandHandlersIterator;

    
        //_____ Attributes __________
    protected:
        static tCommandHandlersList handlers;

        CommunicationChannel    _channel;
        ITask*                  _pTask;
        IPerception*            _pPerception;
    };
}

#endif
