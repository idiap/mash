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


/** @file   sandbox_controller.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxController' class
*/

#include "sandbox_controller.h"
#include "sandbox_messages.h"
#include "declarations.h"
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <vector>
#include <fstream>
#include <errno.h>


using namespace std;
using namespace Mash;
using namespace Mash::SandboxControllerDeclarations;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxController::SandboxController()
: _pid(0), _lastError(ERROR_NONE), _pListener(0), _bJailed(false)
{
    _outStream.setVerbosityLevel(3);
}


SandboxController::~SandboxController()
{
    if (_pid)
        closeSandbox();

    // Delete the log files
    _outStream.deleteFile();

    tLogFileInfosIterator iter, iterEnd;
    for (iter = _logFilesInfos.begin(), iterEnd = _logFilesInfos.end();
         iter != iterEnd; ++iter)
    {
        if (_configuration.bDeleteAllLogFiles || !iter->bKeepFile)
            remove((_configuration.strLogDir + iter->strName + _strLogFileSuffix + ".log").c_str());
    }
}


/***************************** SANDBOX MANAGEMENT *****************************/

bool SandboxController::closeSandbox()
{
    if (!_pid)
        return ERROR_NONE;

    // Delete the core dump file (if any)
    string strFileName = getCoreDumpFileName();
    unlink(strFileName.c_str());

    bool result = false;
    
    _channel.startPacket(SANDBOX_COMMAND_TERMINATE);
    if (_channel.sendPacket() == ERROR_NONE)
        result = waitResponse();

    _pid = 0;

    _outStream.deleteFile();

    return result;
    
}


bool SandboxController::createSandbox(tPluginType pluginType,
                                      const tSandboxConfiguration& configuration,
                                      ISandboxControllerListener* pListener)
{
    // Assertions
    assert(!_pid);

    _outStream << "Creation of the sandbox" << endl;

    _configuration = configuration;
    _pListener = pListener;

    // Create the communication channel
    CommunicationChannel master, slave;
    CommunicationChannel::create(&master, &slave);

    // Compute the suffix of the log files of the sandbox
    time_t t;
    struct tm* timeinfo;
    char buffer[17];

    time(&t);
    timeinfo = localtime(&t);

    strftime(buffer, 17, "_%Y%m%d-%H%M%S", timeinfo);

    _strLogFileSuffix = buffer;

    // Fork the process
    _pid = fork();

    // Child process
    if (_pid == 0)
    {
        // Communication channel setup
        master.close();
        
        // Close all the file handlers that the sandbox doesn't need (note: we keep
        // stdout, stderr and stdin!)
        for (int i = 3; i < getdtablesize(); ++i)
        {
            if ((i != slave.writefd()) && (i != slave.readfd()))
                close(i);
        }

        tStringList vargs;
        
        vargs.push_back("./sandbox");
        vargs.push_back("--readfd=" + StringUtils::toString(slave.readfd()));
        vargs.push_back("--writefd=" + StringUtils::toString(slave.writefd()));

        if (!_configuration.strUsername.empty())
            vargs.push_back("--username=" + _configuration.strUsername);

        if (!_configuration.strLogDir.empty())
            vargs.push_back("--logfolder=" + _configuration.strLogDir);

        vargs.push_back("--logsuffix=" + _strLogFileSuffix);

        if (!_configuration.strOutputDir.empty())
            vargs.push_back("--outputfolder=" + _configuration.strOutputDir);

        if (!_configuration.strJailDir.empty())
            vargs.push_back("--jailfolder=" + _configuration.strJailDir);

        if (_configuration.verbosity == 1)
            vargs.push_back("-v");
        else if (_configuration.verbosity == 2)
            vargs.push_back("-vv");
        else if (_configuration.verbosity == 3)
            vargs.push_back("-vvv");
        else if (_configuration.verbosity == 4)
            vargs.push_back("-vvvv");
        else if (_configuration.verbosity >= 5)
            vargs.push_back("-vvvvv");
        
        switch (pluginType)
        {
            case PLUGIN_CLASSIFIER: vargs.push_back("classifier"); break;
            case PLUGIN_GOALPLANNER: vargs.push_back("goalplanner"); break;
            case PLUGIN_INSTRUMENT: vargs.push_back("instruments"); break;
            case PLUGIN_HEURISTIC: vargs.push_back("heuristics"); break;
        }

        char** cargs = new char*[vargs.size() + 1];
        memset(cargs, 0, (vargs.size() + 1) * sizeof(char*));
        
        for (unsigned int i = 0; i < vargs.size(); ++i)
            cargs[i] = (char*) vargs[i].c_str();

        
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
        const char* sandbox_environment[] = {
            0,
            0
        };

        char* working_dir = getcwd(0, 255);

        string ld_preload = "LD_PRELOAD=";
        ld_preload += working_dir;
        ld_preload += "/libsandbox-warden.so";

        free(working_dir);

        sandbox_environment[0] = ld_preload.c_str();

        // Executes the sandbox program, replacing the current one
        execve("./sandbox", cargs, (char**) sandbox_environment);
#else

        // Executes the sandbox program, replacing the current one
        execvp("./sandbox", cargs);

#endif

        // If this point is reached, the sandbox program wasn't executed.
        // Tell the controller about it
        slave.startPacket(SANDBOX_MESSAGE_CREATION_FAILED);
        slave.sendPacket();

        _exit(0);
    }
    else if (_pid < 0)
    {
        _lastError = ERROR_FORK;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }
    
 
    // Communication channel setup
    slave.close();
    _channel = master;
    master.close();

    // Don't assign an output stream to the channel if not specifically asked to do so
    if (_configuration.verbosity >= 4)
    {
        OutStream channelStream(_outStream);
        channelStream.setVerbosityLevel(4);
        _channel.setOutputStream(channelStream);
    }

    // Wait for a message about the status of the sandbox
    tSandboxMessage status;
    _channel.receivePacket(&status, TIMEOUT_SANDBOX_CREATION);

    if (!_channel.good())
    {
        _pid = 0;
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return false;
    }
    
    if (status != SANDBOX_MESSAGE_CREATION_SUCCESSFUL)
    {
        _pid = 0;
        _lastError = ERROR_SANDBOX_CREATION;
        _outStream << getErrorDescription(_lastError) << endl;
        return false;
    }
        
    return true;
}


/****************************** PLUGINS MANAGEMENT ****************************/

bool SandboxController::setPluginsFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(_pid);
    
    if (getLastError() != ERROR_NONE)
        return false;
    
    _outStream << "< SET_PLUGINS_FOLDER " << strPath << endl;
    
    // Send the command to the sandbox
    _channel.startPacket(SANDBOX_COMMAND_SET_PLUGINS_FOLDER);
    _channel.add(strPath);
    _channel.sendPacket();

    if (!_channel.good())
        return false;

    // Read the response
    return waitResponse();
}


int SandboxController::loadPlugin(const std::string& strName)
{
    // Assertions
    assert(!strName.empty());
    assert(_pid);
    
    if (getLastError() != ERROR_NONE)
        return false;

    _strLastLoadedPlugin = strName;
    
    _outStream << "< LOAD_PLUGIN " << strName << endl;

    // Send the command to the sandbox
    _channel.startPacket(SANDBOX_COMMAND_LOAD_PLUGIN);
    _channel.add(strName);
    _channel.sendPacket();

    if (!_channel.good())
        return false;

    // Read the response
    if (!waitResponse())
        return -1;

    _plugins.push_back(strName);
    
    return _plugins.size() - 1;
}


bool SandboxController::createPlugins()
{
    // Assertions
    assert(_pid);
    
    if (getLastError() != ERROR_NONE)
        return false;

    if (_plugins.empty())
        return true;

    // When doing that, the sandbox enters the 'jailed' state
    _bJailed = true;
    
    _outStream << "< CREATE_PLUGINS " << endl;

    // Send the command to the sandbox
    _channel.startPacket(SANDBOX_COMMAND_CREATE_PLUGINS);
    _channel.sendPacket();

    if (!_channel.good())
        return false;

    // Read the response
    return waitResponse(TIMEOUT_SANDBOX);
}


int SandboxController::getPluginIndex(const std::string& strName) const
{
    int index;
    tStringList::const_iterator iter, iterEnd;
    
    for (index = 0, iter = _plugins.begin(), iterEnd = _plugins.end();
         iter != iterEnd; ++iter, ++index)
    {
        if ((*iter) == strName)
            return index;
    }

    return -1;
}


/*********************************** METHODS **********************************/

bool SandboxController::ping()
{
    // Assertions
    assert(_pid);

    if (_channel.getLastError() != ERROR_NONE)
        return false;

    // Send the command to the sandbox
    _channel.startPacket(SANDBOX_MESSAGE_PING);
    _channel.sendPacket();

    if (!_channel.good())
        return false;

    // Read the response
    return waitResponse(TIMEOUT_SANDBOX);
}


std::string SandboxController::getStackTrace()
{
    // Check that the core dump file exists
    string strFileName = getCoreDumpFileName();

    _outStream << "Trying to retrieve the stack trace from the core dump file '"
               << strFileName << "'" << endl;

    int ret = access(strFileName.c_str(), F_OK);
    if (ret == -1)
    {
        _outStream << "    ERROR - " << strerror(errno) << endl;
        return "";
    }

    _outStream << "    The core dump file is ready" << endl;

    // Process the list of source file directories
    string strSourceDirs;

    _outStream << "    Using the source files located at:" << endl;

    tStringList source_paths = StringUtils::split(_configuration.strSourceDir, ";");
    tStringIterator iter, iterEnd;
    for (iter = source_paths.begin(), iterEnd = source_paths.end(); iter != iterEnd; ++iter)
    {
        if (!strSourceDirs.empty())
            strSourceDirs += " ";
        
        strSourceDirs += "\"" + *iter + "\"";

        _outStream << "        - " << *iter << endl;
    }

    string strStackTrace;

    // Run the core dump analyzer script
    string strCommand = _configuration.strScriptsDir + "coredump_analyzer.py --tempfolder=" +
                        _configuration.strTempDir + " " + strFileName + " " +
                        strSourceDirs;
    FILE* analyzer = popen(strCommand.c_str(), "r");

    if (!analyzer)
    {
        _outStream << "   ERROR - Failed to launch the core dump analyzer" << endl;
        return "";
    }

    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    while (true)
    {
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(fileno(analyzer), &readfds);

        int nb = select(fileno(analyzer) + 1, &readfds, 0, 0, 0);
        if (nb == -1)
        {
            if (errno == EINTR)
                continue;

            _outStream << "   ERROR in select(): " << strerror(errno) << endl;
            break;
        }            
        
        memset(buffer, 0, BUFFER_SIZE);

        size_t count = fread(buffer, sizeof(char), BUFFER_SIZE, analyzer);
        if (count > 0)
            strStackTrace += buffer;

        if (feof(analyzer))
            break;
    }

    pclose(analyzer);

    if (strStackTrace.empty() || StringUtils::startsWith(strStackTrace, "ERROR -"))
    {
        _outStream << "   ERROR - Failed to analyze the core dump" << endl;
        
        if (StringUtils::startsWith(strStackTrace, "ERROR -"))
        {
            _outStream << "       Got: " << strStackTrace << endl;
            strStackTrace = "";
        }
    }

    return strStackTrace;
}


int SandboxController::getLogFileContent(unsigned int index, std::string& strName,
                                         unsigned char** pBuffer, int max_size)
{
    if (!_outStream.getFileName().empty())
    {
        if (index == 0)
        {
            strName = _outStream.getName();
            return _outStream.dump(pBuffer, max_size);
        }
        --index;
    }
    
    if (index >= _logFilesInfos.size())
    {
        *pBuffer = 0;
        return 0;
    }

    strName = _logFilesInfos[index].strName;
    return getFileContent(_configuration.strLogDir + strName +
                          _strLogFileSuffix + ".log", pBuffer,
                          (_logFilesInfos[index].bNoLimit ? 0 : max_size));
}


int SandboxController::getFileContent(const std::string& strFileName, unsigned char** pBuffer, int max_size)
{
    // Assertions
    assert(!strFileName.empty());
    assert(max_size >= 0);
    
    ifstream inFile;
    inFile.open(strFileName.c_str());
    if (!inFile.is_open())
        return 0;
    
    inFile.seekg(0, ios_base::end);
    int size = inFile.tellg();

    if (size <= 0)
        return 0;

    if ((max_size > 0) && (max_size < 100))
        max_size = 100;

    bool bTruncated = false;

    if ((max_size > 0) && (size > max_size))
    {
        inFile.seekg(size - (max_size - 4), ios_base::beg);
        size = max_size;
        bTruncated = true;
    }
    else
    {
        inFile.seekg(0, ios_base::beg);
    }

    (*pBuffer) = new unsigned char[size + 1];
    memset(*pBuffer, 0, (size + 1) * sizeof(unsigned char));

    unsigned char* pDest = (*pBuffer);

    if (bTruncated)
    {
        memcpy(pDest, "...\n", 4);
        pDest += 4;
    }

    while (size - (pDest - (*pBuffer)) > 0)
    {
        inFile.read((char*) pDest, size - (pDest - (*pBuffer)));
        unsigned int nb = inFile.gcount();
        pDest += nb;
    }

    inFile.close();
    
    return size;
}


std::string SandboxController::getCoreDumpFileName() const
{
    // Assertions
    assert(_pid);

    // Compute the actual name of the core dump file
    string strFileName = StringUtils::replaceAll(_configuration.strCoreDumpTemplate, "$PID",
                                                 StringUtils::toString(_pid));
    
    if (_bJailed)
    {
        if (!StringUtils::startsWith(strFileName, "/") && !_configuration.strJailDir.empty())
            strFileName = _configuration.strJailDir + strFileName;
    }

    return strFileName;
}


bool SandboxController::waitResponse(unsigned int timeout)
{
    // Assertions
    assert(_pid);
    
    // Declarations
    tSandboxMessage message;
    
    while (_channel.good())
    {
        if (_channel.receivePacket(&message, timeout) != ERROR_NONE)
            break;

        if (message == SANDBOX_MESSAGE_KEEP_ALIVE)
            continue;
        
        if ((message == SANDBOX_MESSAGE_RESPONSE) || (message == SANDBOX_MESSAGE_PONG))
            return true;

        if (message == SANDBOX_MESSAGE_ERROR)
        {
            _channel.read(&_lastError);
            
            if (_lastError == ERROR_WARDEN)
            {
                _channel.read(&_lastErrorDetails);
                _outStream << "> ERROR " << _lastErrorDetails << endl;
            }
            else
            {
                _outStream << "> ERROR " << getErrorDescription(_lastError) << endl;
            }
            
            return false;
        }

        if (message == SANDBOX_EVENT_MEMORY_LIMIT_REACHED)
        {
            _lastError = ERROR_SANDBOX_MEMORY_LIMIT_REACHED;
            _outStream << "> ERROR " << getErrorDescription(_lastError) << endl;
            return false;
        }

        if (message == SANDBOX_EVENT_FORBIDDEN_SYSTEM_CALL)
        {
            _lastError = ERROR_SANDBOX_FORBIDDEN_SYSTEM_CALL;
            _channel.read(&_lastErrorDetails);
            
            _outStream << "> ERROR " << getErrorDescription(_lastError) << ": "
                       << _lastErrorDetails << endl;
            return false;
        }

        if (!_pListener)
        {
            _outStream << "Unknown message received: " << message << endl;
            return false;
        }
        
        tCommandProcessingResult result = _pListener->processResponse(message);
        if (result != COMMAND_PROCESSED)
        {
            switch (result)
            {
                case SOURCE_PLUGIN_CRASHED:
                    if (_pluginType == PLUGIN_CLASSIFIER)
                        _lastError = ERROR_CLASSIFIER_CRASHED;
                    else if (_pluginType == PLUGIN_GOALPLANNER)
                        _lastError = ERROR_PLANNER_CRASHED;
                    else if (_pluginType == PLUGIN_INSTRUMENT)
                        _lastError = ERROR_INSTRUMENT_CRASHED;
                    else
                        _lastError = ERROR_HEURISTIC_CRASHED;
                    break;

                case DEST_PLUGIN_CRASHED:   _lastError = ERROR_HEURISTIC_CRASHED; break;
                case DEST_PLUGIN_TIMEOUT:   _lastError = ERROR_HEURISTIC_TIMEOUT; break;

                case INVALID_ARGUMENTS:
                case APPSERVER_ERROR:
                    _lastError = ERROR_CHANNEL_PROTOCOL;
                    break;
                    
                default:
                    _lastError = ERROR_CHANNEL_UNEXPECTED_RESPONSE;
                    break;
            }

            _outStream << getErrorDescription(_lastError) << ", last message received = "
                       << message << endl;

            return false;
        }
    }
    
    // Error handling
    return false;
}
