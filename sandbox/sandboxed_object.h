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


/** @file   sandboxed_object.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'ISandboxedObject' interface
*/

#ifndef _ISANDBOXEDOBJECT_H_
#define _ISANDBOXEDOBJECT_H_

#include <mash-utils/platform.h>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include "warden.h"
#else
    #include "no_warden.h"
#endif

#include <mash-sandboxing/communication_channel.h>
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-utils/data_writer.h>
#include <mash-utils/outstream.h>
#include <mash-utils/declarations.h>
#include <mash-utils/errors.h>
#include <mash/predictor_model.h>
#include <sys/times.h>
#include <sys/time.h>
#include <signal.h>
#include <string>
#include <assert.h>


//------------------------------------------------------------------------------
/// @brief  Interface for all the sandboxed objects
//------------------------------------------------------------------------------
class ISandboxedObject
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    //--------------------------------------------------------------------------
    ISandboxedObject(const Mash::CommunicationChannel& channel,
                     Mash::OutStream* pOutStream);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~ISandboxedObject();


    //_____ Methods to implement __________
public:
    virtual Mash::tError setPluginsFolder(const std::string& strPath) = 0;
    virtual Mash::tError loadPlugin(const std::string& strName) = 0;
    virtual Mash::tError createPlugins(Mash::OutStream* pOutStream,
                                       const std::vector<Mash::DataWriter>& dataWriters,
                                       const std::vector<Mash::DataWriter>& outCache,
                                       const std::vector<Mash::DataReader>& inCache,
                                       const Mash::PredictorModel& inModel,
                                       const Mash::DataReader& inInternalData,
                                       const Mash::PredictorModel& outModel,
                                       const Mash::DataWriter& outInternalData) = 0;
    virtual void handleCommand(Mash::tSandboxMessage command) = 0;


    //_____ Warden-related methods __________
protected:
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    static void warden_listener(tWardenContext* pContext, tWardenStatus status,
                                const char* details);
#endif


    //_____ Attributes __________
protected:
    static ISandboxedObject* pInstance;
    
    Mash::CommunicationChannel  _channel;
    Mash::OutStream             _outStream;
};

#endif
