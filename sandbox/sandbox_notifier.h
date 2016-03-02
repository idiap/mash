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


/** @file   sandbox_notifier.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxNotifier' class
*/

#ifndef _MASH_SANDBOXNOTIFIER_H_
#define _MASH_SANDBOXNOTIFIER_H_

#include <mash-utils/platform.h>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include "warden.h"
#else
    #include "no_warden.h"
#endif

#include <mash/notifier_interface.h>
#include <mash-sandboxing/communication_channel.h>
#include <mash-utils/outstream.h>


//------------------------------------------------------------------------------
/// @brief  Implementation of the Notifier object used by the sandboxed
///         predictors to send notifications to the real Notifier object,
///         located in the calling process
//------------------------------------------------------------------------------
class MASH_SYMBOL SandboxNotifier: public Mash::INotifier
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    ///
    /// @param  channel         The communication channel to use to communicate
    ///                         with the calling process
    /// @param  pOutStream      The output stream to use (can be 0)
    /// @param  pWardenContext  The warden context to use
    //--------------------------------------------------------------------------
    SandboxNotifier(const Mash::CommunicationChannel& channel,
                    Mash::OutStream* pOutStream,
                    tWardenContext* pWardenContext);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~SandboxNotifier();


    //_____ Notifications __________
public:
    //--------------------------------------------------------------------------
    /// @copy   Mash::INotifier::onTrainingStepDone
    //--------------------------------------------------------------------------
    virtual void onTrainingStepDone(unsigned int step,
                                    unsigned int nbTotalSteps = 0);


    //_____ Attributes __________
protected:
    Mash::CommunicationChannel _channel;
    Mash::OutStream            _outStream;
    tWardenContext*            _pWardenContext;
};

#endif
