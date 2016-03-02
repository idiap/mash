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


/** @file   sandbox_notifier.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxNotifier' class
*/

#include "sandbox_notifier.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxNotifier::SandboxNotifier(const CommunicationChannel& channel,
                                 OutStream* pOutStream,
                                 tWardenContext* pWardenContext)
: _channel(channel), _pWardenContext(pWardenContext)
{
    if (pOutStream)
        _outStream = *pOutStream;
}


SandboxNotifier::~SandboxNotifier()
{
}


/****************************** NOTIFICATIONS *********************************/

void SandboxNotifier::onTrainingStepDone(unsigned int step, unsigned int nbTotalSteps)
{
    tWardenContext* pPreviousContext = getWardenContext();
    setWardenContext(0);

    _outStream << "< NOTIFICATION_TRAINING_STEP_DONE " << step << " " << nbTotalSteps << endl;

    // Send the notification to the child
    _channel.startPacket(SANDBOX_NOTIFICATION_TRAINING_STEP_DONE);
    _channel.add((int) step);
    _channel.add((int) nbTotalSteps);
    _channel.sendPacket();

    setWardenContext(pPreviousContext);
}
