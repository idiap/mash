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


/** @file   sandbox.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'Sandbox' class
*/

#include "sandbox.h"
#include "sandboxed_heuristics.h"
#include "sandboxed_classifier.h"
#include "sandboxed_planner.h"
#include "sandboxed_instruments.h"
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/time.h> 
#include <sys/resource.h>
#include <pwd.h>
#include <memory.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <iostream>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include <sys/prctl.h>
#endif


using namespace std;
using namespace Mash;


/****************************** STATIC ATTRIBUTES *****************************/

Sandbox::tCommandHandlersList Sandbox::handlers;


/************************* CONSTRUCTION / DESTRUCTION *************************/

Sandbox::Sandbox()
: _pSandboxedObject(0), _user(0)
{
    if (handlers.empty())
    {
        handlers[SANDBOX_COMMAND_SET_PLUGINS_FOLDER]    = &Sandbox::handleSetPluginsFolderCommand;
        handlers[SANDBOX_COMMAND_LOAD_PLUGIN]           = &Sandbox::handleLoadPluginCommand;
        handlers[SANDBOX_COMMAND_USE_MODEL]             = &Sandbox::handleUseModelCommand;
        handlers[SANDBOX_COMMAND_CREATE_PLUGINS]        = &Sandbox::handleCreatePluginsCommand;
    }
}


Sandbox::~Sandbox()
{
}


/********************************* METHODS ************************************/

bool Sandbox::init(const tConfiguration& configuration)
{
    // Assertions
    assert(!_pSandboxedObject);
    
    _configuration = configuration;
    
    // Check the paths
    _configuration.strLogFolder     = checkPath(_configuration.strLogFolder);
    _configuration.strOutputFolder  = checkPath(_configuration.strOutputFolder);
    _configuration.strJailFolder    = checkPath(_configuration.strJailFolder);

    // Open the log file
    OutStream::verbosityLevel = _configuration.verbosity;

    switch (_configuration.kind)
    {
        case KIND_HEURISTICS:
            _outStream.open("HeuristicsSandbox", _configuration.strLogFolder + "HeuristicsSandbox" + _configuration.strLogSuffix + ".log", 200 * 1024, false);
            break;

        case KIND_CLASSIFIER:
            _outStream.open("PredictorSandbox", _configuration.strLogFolder + "PredictorSandbox" + _configuration.strLogSuffix + ".log", 200 * 1024, false);
            break;

        case KIND_GOALPLANNER:
            _outStream.open("PredictorSandbox", _configuration.strLogFolder + "PredictorSandbox" + _configuration.strLogSuffix + ".log", 200 * 1024, false);
            break;

        case KIND_INSTRUMENTS:
            _outStream.open("InstrumentsSandbox", _configuration.strLogFolder + "InstrumentsSandbox" + _configuration.strLogSuffix + ".log", 200 * 1024, false);
            break;

        case KIND_NONE:
            _outStream.open("Sandbox", _configuration.strLogFolder + "Sandbox" + _configuration.strLogSuffix + ".log", 200 * 1024, false);
        break;
    }

    _outStream.setVerbosityLevel(3);

    // Retrieve the current working folder
    char buffer[256];
    _strWorkingFolder = getcwd(buffer, 256);

    // Check the file handles
    if ((_configuration.read_pipe < 0) || (_configuration.write_pipe <= 0))
    {
        _outStream << "ERROR: Invalid file handles" << endl;
        _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
        _channel.sendPacket();
        return false;
    }

    // Don't assign an output stream to the channel if not specifically asked to do so
    if (_configuration.verbosity >= 4)
    {
        OutStream channelStream(_outStream);
        channelStream.setVerbosityLevel(4);
        _channel.setOutputStream(channelStream);
    }

    // Create the communication channel with the calling process
    _channel.open(CommunicationChannel::ENDPOINT_SLAVE,
                  _configuration.write_pipe, _configuration.read_pipe);

    // Enable core dumps
    struct rlimit limit;
    getrlimit(RLIMIT_CORE, &limit);
    limit.rlim_cur = limit.rlim_max;
    setrlimit(RLIMIT_CORE, &limit);
    
    // Retrieve the user ID
    uid_t uid = geteuid();
    struct passwd* infos = 0;
    
    // Jailing (if super-user)
    if (uid == 0)
    {
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
        int ret = prctl(PR_SET_DUMPABLE, 1);
        if (ret != 0)
        {
            _outStream << "ERROR: Failed to enable the core dumps at startup" << endl;

            _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
            _channel.sendPacket();

            return false;
        }
#endif

        if (_configuration.strJailFolder.empty())
        {
            _outStream << "ERROR: Jail folder required" << endl;

            _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
            _channel.sendPacket();

            return false;
        }

        if (_configuration.strUsername.empty())
        {
            _outStream << "ERROR: Username required" << endl;

            _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
            _channel.sendPacket();

            return false;
        }

        infos = getpwnam(_configuration.strUsername.c_str());
        if (!infos)
        {
            _outStream << "ERROR: Unknown username" << endl;

            _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
            _channel.sendPacket();

            return false;
        }
        
        _user = infos->pw_uid;
    }

    // Create the jail folder if necessary
    size_t offset = _configuration.strJailFolder.find_last_of("/");
    if (offset != string::npos)
    {
        string dest = _configuration.strJailFolder.substr(0, offset + 1);
    
        mode_t previous_mode = umask(0);
    
        size_t start = dest.find("/", 0);
        while (start != string::npos)
        {
            string dirname = dest.substr(0, start);
                    
            DIR* d = opendir(dirname.c_str());
            if (!d)
                mkdir(dirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH);
            else
                closedir(d);

            start = dest.find("/", start + 1);
        }

        umask(previous_mode);
    }

    // Create the sandboxed object
    switch (_configuration.kind)
    {
        case KIND_HEURISTICS:
            _pSandboxedObject = new SandboxedHeuristics(_channel, &_outStream);
            break;

        case KIND_CLASSIFIER:
            _pSandboxedObject = new SandboxedClassifier(_channel, &_outStream);
            break;

        case KIND_GOALPLANNER:
            _pSandboxedObject = new SandboxedPlanner(_channel, &_outStream);
            break;

        case KIND_INSTRUMENTS:
            _pSandboxedObject = new SandboxedInstruments(_channel, &_outStream);
            break;

        case KIND_NONE:
            _outStream << "ERROR: Unsupported type of sandbox" << endl;
            _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
            _channel.sendPacket();
            return false;
    }

    // Tell the calling process that everything went OK
    _channel.startPacket(SANDBOX_MESSAGE_CREATION_SUCCESSFUL);
    _channel.sendPacket();

    return _channel.good();
}


bool Sandbox::run()
{
    // Assertions
    assert(_pSandboxedObject);
    
    // Declarations
    bool            bFinished   = false;
    tSandboxMessage command;

    // Process the commands from the calling process
    while (!bFinished && _channel.good())
    {
        if (_channel.receivePacket(&command) != ERROR_NONE)
            break;
        
        if (command == SANDBOX_COMMAND_TERMINATE)
        {
            bFinished = true;
            break;
        }

        if (command == SANDBOX_MESSAGE_PING)
        {
            _channel.startPacket(SANDBOX_MESSAGE_PONG);
            _channel.sendPacket();
            continue;
        }

        tCommandHandlersIterator iter = handlers.find(command);
        if (iter != handlers.end())
        {
            tCommandHandler handler = iter->second;
            tError result = (this->*handler)();
            
            if (result != ERROR_NONE)
            {
                _outStream << "< ERROR " << getErrorDescription(result) << endl;

                _channel.startPacket(SANDBOX_MESSAGE_ERROR);
                _channel.add(result);
                _channel.sendPacket();
            }
        }
        else if (_pSandboxedObject)
        {
            _pSandboxedObject->handleCommand(command);
        }
        else
        {
            _channel.startPacket(SANDBOX_MESSAGE_UNKNOWN_COMMAND);
            _channel.sendPacket();
        }
    }

    // Cleanup
    delete _pSandboxedObject;

    // If the parent asked to terminate the process, send some data to let it
    // know everything is OK
    if (bFinished)
    {
        _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
        _channel.sendPacket();
    }

    return _channel.good();
}


std::string Sandbox::checkPath(const std::string& strPath) const
{
    string newPath = strPath;

    if (newPath.empty())
        return newPath;

    newPath = StringUtils::replaceAll(newPath, "./", "");

    if (newPath[0] != '/')
    {
        tStringList parts = StringUtils::split(newPath, "/");
        newPath = "";
        for (unsigned int i = 1; i < parts.size(); ++i)
        {
            if (parts[i - 1].empty())
                continue;
            
            if (parts[i] != "..")
                newPath += parts[i - 1] + "/";
            else
                ++i;
        }
        
        if (!parts[parts.size() - 1].empty() && (parts[parts.size() - 1] != ".."))
            newPath += parts[parts.size() - 1] + "/";
    }

    if (newPath[newPath.size() - 1] != '/')
        newPath += "/";

    return newPath;
}


/**************************** COMMAND HANDLERS ********************************/

tError Sandbox::handleSetPluginsFolderCommand()
{
    // Assertions
    assert(_pSandboxedObject);
    
    // Retrieve the path
    string strPath;
    if (!_channel.read(&strPath))
        return _channel.getLastError();

    // Tell the sandboxed object about it
    tError result = _pSandboxedObject->setPluginsFolder(strPath);
    if (result != ERROR_NONE)
        return result;

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return _channel.getLastError();
}


tError Sandbox::handleLoadPluginCommand()
{
    // Assertions
    assert(_pSandboxedObject);
    
    // Retrieve the name of the plugin
    string strName;
    if (!_channel.read(&strName))
        return _channel.getLastError();

    // Tell the sandboxed object about it
    tError result = _pSandboxedObject->loadPlugin(strName);
    if (result != ERROR_NONE)
        return result;
        
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return _channel.getLastError();
}


tError Sandbox::handleUseModelCommand()
{
    // Assertions
    assert(_pSandboxedObject);
    assert(_strModelFile.empty());
    assert(_strInternalDataFile.empty());
    assert((_configuration.kind == KIND_CLASSIFIER) || (_configuration.kind == KIND_GOALPLANNER));
    
    // Retrieve the model name
    if (!_channel.read(&_strModelFile) || !_channel.read(&_strInternalDataFile))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> USE_MODEL " << _strModelFile << " " << _strInternalDataFile << endl;
    
    if (_strModelFile == "-")
    {
        _strModelFile = "";
        _strInternalDataFile = "";
    }
    else if (_strInternalDataFile == "-")
    {
        _strInternalDataFile = "";
    }
    
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError Sandbox::handleCreatePluginsCommand()
{
    // Assertions
    assert(_pSandboxedObject);
    
    // If needed, create the output stream of the sandboxed object
    OutStream sandboxedObjectOutStream;
    string strPredictorLogFileName = _configuration.strLogFolder + "Predictor" + _configuration.strLogSuffix + ".log";
    
    if (_configuration.kind == KIND_CLASSIFIER)
        sandboxedObjectOutStream.open("Classifier", strPredictorLogFileName, 10 * 1024 * 1024, false);
    else if (_configuration.kind == KIND_GOALPLANNER)
        sandboxedObjectOutStream.open("Goal-planner", strPredictorLogFileName, 10 * 1024 * 1024, false);

    sandboxedObjectOutStream.setVerbosityLevel(1);

    // If needed, create the data writers of the sandboxed objects
    std::vector<DataWriter> dataWriters;
    
    if (_configuration.kind == KIND_INSTRUMENTS)
    {
        SandboxedInstruments* pInstruments = dynamic_cast<SandboxedInstruments*>(_pSandboxedObject);
    
        for (unsigned int i = 0; i < pInstruments->nbInstruments(); ++i)
        {
            DataWriter writer;
            writer.open(_configuration.strOutputFolder + pInstruments->instrumentName(i) + ".data");
            dataWriters.push_back(writer);
        }
    }
    else if ((_configuration.kind == KIND_CLASSIFIER) || (_configuration.kind == KIND_GOALPLANNER))
    {
        DataWriter writer;
        writer.open(_configuration.strOutputFolder + "predictor.data");
        dataWriters.push_back(writer);
    }

    // If needed, create the cache writers and readers of the sandboxed objects
    std::vector<DataWriter> cacheWriters;
    std::vector<DataReader> cacheReaders;
    
    if ((_configuration.kind == KIND_CLASSIFIER)) // || (_configuration.kind == KIND_GOALPLANNER))
    {
        DataWriter writer;
        writer.open(_configuration.strJailFolder + "predictor.cache");
        cacheWriters.push_back(writer);

        DataReader reader;
        reader.open(_configuration.strJailFolder + "predictor.cache");
        cacheReaders.push_back(reader);
    }

    // Load the input model
    PredictorModel inModel;
    DataReader inInternalData;
    
    if (!_strModelFile.empty())
    {
        if (inModel.open(_strModelFile) && !_strInternalDataFile.empty())
            inInternalData.open(_strInternalDataFile);
    }

    // Setup the output model
    PredictorModel outModel;
    DataWriter outInternalData;
    
    if ((_configuration.kind == KIND_CLASSIFIER) || (_configuration.kind == KIND_GOALPLANNER))
    {
        outModel.create(_configuration.strOutputFolder + "predictor.model");
        outInternalData.open(_configuration.strOutputFolder + "predictor.internal");
    }

    // Switch to the jail folder
    if (!_configuration.strJailFolder.empty())
        chdir(_configuration.strJailFolder.c_str());

    uid_t uid = geteuid();
    if (uid == 0)
    {
        int ret = chroot(".");
        if (ret != 0)
        {
            _outStream << "ERROR: Failed to create the jail" << endl;

            _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
            _channel.sendPacket();

            return ERROR_SANDBOX_CREATION;
        }
    }

    // Do not allow the process to create any file handler from now on
    struct rlimit limit;
    limit.rlim_cur = 0;
    limit.rlim_max = 0;
    setrlimit(RLIMIT_NOFILE, &limit);

    // Switch to the sandboxed user
    if (uid == 0)
    {
        int ret = setuid(_user);
        if (ret != 0)
        {
            _outStream << "ERROR: Failed to change the user, reason: " << strerror(errno) << endl;

            _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
            _channel.sendPacket();

            return ERROR_SANDBOX_CREATION;
        }

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
        ret = prctl(PR_SET_DUMPABLE, 1);
        if (ret != 0)
        {
            _outStream << "ERROR: Failed to enable the core dumps after the switching of user" << endl;

            _channel.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
            _channel.sendPacket();

            return ERROR_SANDBOX_CREATION;
        }
#endif
    }

    // No subprocess
    limit.rlim_cur = 0;
    limit.rlim_max = 0;
    setrlimit(RLIMIT_NPROC, &limit);

    // Tell the sandboxed object to create the plugins
    tError result = _pSandboxedObject->createPlugins(&sandboxedObjectOutStream, dataWriters,
                                                     cacheWriters, cacheReaders,
                                                     inModel, inInternalData,
                                                     outModel, outInternalData);

    if (result != ERROR_NONE)
        return result;
        
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return _channel.getLastError();
}
