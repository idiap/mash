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


/** @file   sandbox_notifier_proxy.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxNotifierProxy' class
*/

#include "sandbox_notifier_proxy.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-classification/classifier_input_set.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <stdlib.h>


using namespace std;
using namespace Mash;
using namespace Mash::SandboxControllerDeclarations;


/****************************** STATIC ATTRIBUTES *****************************/

SandboxNotifierProxy::tNotificationHandlersList SandboxNotifierProxy::handlers;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxNotifierProxy::SandboxNotifierProxy(INotifier* pNotifier,
                                           const CommunicationChannel& channel)
: _channel(channel), _pNotifier(pNotifier)
{
    if (handlers.empty())
    {
        handlers[SANDBOX_NOTIFICATION_TRAINING_STEP_DONE] = &SandboxNotifierProxy::handleTrainingStepDoneNotification;
    }
}


SandboxNotifierProxy::~SandboxNotifierProxy()
{
}


/**************************** COMMAND HANDLERS ********************************/

tCommandProcessingResult SandboxNotifierProxy::processResponse(tSandboxMessage message)
{
    // Assertions
    assert(!handlers.empty());

    tNotificationHandlersIterator iter = handlers.find(message);
    if (iter != handlers.end())
    {
        tNotificationHandler handler = iter->second;
        return (this->*handler)();
    }

    return COMMAND_UNKNOWN;
}


tCommandProcessingResult SandboxNotifierProxy::handleTrainingStepDoneNotification()
{
    // Retrieve the parameters
    unsigned int step, nbTotalSteps;
    if (!_channel.read(&step) || !_channel.read(&nbTotalSteps))
        return SOURCE_PLUGIN_CRASHED;

    // Inform the real notifier
    if (_pNotifier)
        _pNotifier->onTrainingStepDone(step, nbTotalSteps);
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}
