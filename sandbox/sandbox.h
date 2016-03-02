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


/** @file   sandbox.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Sandbox' class
*/

#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include "sandboxed_object.h"
#include <mash-sandboxing/communication_channel.h>
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-utils/outstream.h>
#include <string>
#include <map>


//--------------------------------------------------------------------------
/// @brief  Represents the sandbox
//--------------------------------------------------------------------------
class Sandbox
{
    //_____ Internal types __________
public:
    enum tKind
    {
        KIND_NONE,
        KIND_HEURISTICS,
        KIND_CLASSIFIER,
        KIND_GOALPLANNER,
        KIND_INSTRUMENTS,
    };
    
    
    struct tConfiguration
    {
        tConfiguration()
        : kind(KIND_NONE), strUsername(""), strLogFolder("logs"),
          strOutputFolder("out"), strJailFolder("jail"), read_pipe(0),
          write_pipe(0), verbosity(0)
        {
        }        

        tKind           kind;
        std::string     strUsername;
        std::string     strLogFolder;
        std::string     strLogSuffix;
        std::string     strOutputFolder;
        std::string     strJailFolder;
        int             read_pipe;
        int             write_pipe;
        unsigned int    verbosity;
    };


    //_____ Construction / Destruction __________
public:
    //----------------------------------------------------------------------
    /// @brief  Constructor
    //----------------------------------------------------------------------
    Sandbox();

    //----------------------------------------------------------------------
    /// @brief  Destructor
    //----------------------------------------------------------------------
    ~Sandbox();


    //_____ Methods __________
public:
    bool init(const tConfiguration& configuration);
    bool run();

private:
    std::string checkPath(const std::string& strPath) const;


    //_____ Command handlers __________
private:
    Mash::tError handleSetPluginsFolderCommand();
    Mash::tError handleLoadPluginCommand();
    Mash::tError handleUseModelCommand();
    Mash::tError handleCreatePluginsCommand();


    //_____ Internal types __________
private:
    typedef Mash::tError (Sandbox::*tCommandHandler)();

    typedef std::map<Mash::tSandboxMessage, tCommandHandler>    tCommandHandlersList;
    typedef tCommandHandlersList::iterator                      tCommandHandlersIterator;
    

    //_____ Attributes __________
private:
    static tCommandHandlersList handlers;

    tConfiguration              _configuration;
    std::string                 _strWorkingFolder;
    uid_t                       _user;
    Mash::CommunicationChannel  _channel;
    Mash::OutStream             _outStream;
    ISandboxedObject*           _pSandboxedObject;
    std::string                 _strModelFile;
    std::string                 _strInternalDataFile;
};

#endif
