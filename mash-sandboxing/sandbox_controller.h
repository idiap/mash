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


/** @file   sandbox_controller.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxController' class
*/

#ifndef _MASH_SANDBOXCONTROLLER_H_
#define _MASH_SANDBOXCONTROLLER_H_

#include <mash-utils/declarations.h>
#include <mash-utils/outstream.h>
#include "sandbox_controller_listener.h"
#include "communication_channel.h"
#include "sandbox_messages.h"
#include <assert.h>


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Controller for a sandboxed environement when untrusted plugins
    ///         can be safely executed
    ///
    /// The management of the sandbox itself is delegated to an external
    /// software ('sandbox'), launched by the controller. The controller sends
    /// commands to the sandbox using pipes.
    ///
    /// It is expected that this class is inherited for each kind of sandboxed
    /// object, in order to provide methods mimicking the interface of the
    /// object.
    //--------------------------------------------------------------------------
    class MASH_SYMBOL SandboxController
    {
        //_____ Internal types __________
    private:
        struct tLogFileInfos
        {
            std::string strName;
            bool        bNoLimit;
            bool        bKeepFile;
        };
        
        typedef std::vector<tLogFileInfos>  tLogFileInfosList;
        typedef tLogFileInfosList::iterator tLogFileInfosIterator;
        
        
        //_____ Construction / Destruction __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Constructor
        //----------------------------------------------------------------------
        SandboxController();

        //----------------------------------------------------------------------
        /// @brief  Destructor
        //----------------------------------------------------------------------
        ~SandboxController();


        //_____ Sandbox management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Create a sandbox of the given type
        ///
        /// @param  pluginType      Type of the plugins in the sandbox
        /// @param  configuration   Configuration of the sandbox
        /// @param  pListener       Listener to use (might be 0)
        /// @return                 Error code
        ///
        /// @remark The sandbox controller doesn't take the ownership of the
        ///         listener
        //----------------------------------------------------------------------
        bool createSandbox(tPluginType pluginType,
                           const tSandboxConfiguration& configuration,
                           ISandboxControllerListener* pListener = 0);

        //----------------------------------------------------------------------
        /// @brief  Close the sandbox
        /// @return Error code
        //----------------------------------------------------------------------
        bool closeSandbox();
        

        //_____ Plugins management __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Tell the sandbox about the folder into which the plugins are
        ///         located
        ///
        /// @param  strPath     Path of the folder containing the plugins
        /// @return             'true' if successful
        //----------------------------------------------------------------------
        bool setPluginsFolder(const std::string& strPath);

        //----------------------------------------------------------------------
        /// @brief  Tell the sandbox to load a specific plugin
        ///
        /// @param  strName     Name of the plugin
        /// @return             The index of the plugin, -1 in case of error
        //----------------------------------------------------------------------
        int loadPlugin(const std::string& strName);

        //----------------------------------------------------------------------
        /// @brief  Tell the sandbox to create instances of the objects defined
        ///         in the loaded plugins
        ///
        /// @return 'true' if successful
        //----------------------------------------------------------------------
        bool createPlugins();

        //----------------------------------------------------------------------
        /// @brief  Returns the list of loaded plugins
        //----------------------------------------------------------------------
        inline tStringList getPluginsList() const
        {
            return _plugins;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of loaded plugins
        //----------------------------------------------------------------------
        inline unsigned int nbPlugins() const
        {
            return _plugins.size();
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the index of a plugin
        ///
        /// @param  strName     Name of the plugin
        /// @return             The index of the plugin, -1 if not found
        //----------------------------------------------------------------------
        int getPluginIndex(const std::string& strName) const;

        //----------------------------------------------------------------------
        /// @brief  Returns the name of a plugin
        ///
        /// @param  index   Index of the plugin
        /// @return         The name of the plugin
        //----------------------------------------------------------------------
        inline std::string getPluginName(int index) const
        {
            assert(index < (signed) _plugins.size());

            if (index > -1)
                return _plugins[index];
            else
                return _strLastLoadedPlugin;
        }


        //_____ Methods __________
    public:
        //----------------------------------------------------------------------
        /// @brief  Ping the sandbox
        //----------------------------------------------------------------------
        bool ping();

        inline void setOutputStream(const OutStream& outStream)
        {
            _outStream = outStream;
        }

        inline OutStream& outStream()
        {
            return _outStream;
        }

        inline CommunicationChannel* channel()
        {
            return &_channel;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the last error that occured
        //----------------------------------------------------------------------
        inline tError getLastError()
        {
            return (_lastError != ERROR_NONE ? _lastError : _channel.getLastError());
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the details associated with the last error that
        ///         occured
        //----------------------------------------------------------------------
        inline std::string getLastErrorDetails() const
        {
            return _lastErrorDetails;
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the stacktrace of the sandboxed object (used to
        ///         report debugging informations after a crash)
        //----------------------------------------------------------------------
        std::string getStackTrace();

        //----------------------------------------------------------------------
        /// @brief  Add infos about a log file
        //----------------------------------------------------------------------
        inline void addLogFileInfos(const std::string& strName,
                                    bool bNoLimit = false,
                                    bool bKeepFile = false)
        {
            tLogFileInfos infos;
            infos.strName = strName;
            infos.bNoLimit = bNoLimit;
            infos.bKeepFile = bKeepFile;
            
            _logFilesInfos.push_back(infos);
        }

        //----------------------------------------------------------------------
        /// @brief  Returns the number of log files available
        //----------------------------------------------------------------------
        inline unsigned int getNbLogFiles() const
        {
            return _logFilesInfos.size() + (_outStream.getFileName().empty() ? 0 : 1);
        }

        //----------------------------------------------------------------------
        /// @brief  Return the content of a log file
        ///
        /// @param      index       Index of the log file
        /// @param[out] strName     Name of the log file
        /// @param[out] pBuffer     Buffer holding the content of the file
        /// @param      max_size    Maximum size of the buffer (only the end of
        ///                         the content is in the buffer if necessary)
        /// @return                 The size of the buffer
        //----------------------------------------------------------------------
        int getLogFileContent(unsigned int index, std::string& strName,
                              unsigned char** pBuffer, int max_size = 0);


        //----------------------------------------------------------------------
        /// @brief  Wait for a response from the sandbox
        ///
        /// @param  timeout Delay before which the sandbox must send a response,
        ///                 in milliseconds. If 0, no timeout is used.
        /// @return 'false' in case of failure
        //----------------------------------------------------------------------
        bool waitResponse(unsigned int timeout = 0);

    private:
        //----------------------------------------------------------------------
        /// @brief  Returns the name of the file containing the core dump of the
        ///         sandboxed object
        //----------------------------------------------------------------------
        std::string getCoreDumpFileName() const;
        
        //----------------------------------------------------------------------
        /// @brief  Return the content of a file
        ///
        /// @param      strFileName Name of the file
        /// @param[out] pBuffer     Buffer holding the content of the file
        /// @param      max_size    Maximum size of the buffer (only the end of
        ///                         the content is in the buffer if necessary)
        /// @return                 The size of the buffer
        //----------------------------------------------------------------------
        int getFileContent(const std::string& strFileName, unsigned char** pBuffer,
                           int max_size = 0);


        //_____ Attributes __________
    private:
        OutStream                   _outStream;             ///< Output stream to use for logging
        CommunicationChannel        _channel;               ///< Channel used to communicate with the sandbox
        pid_t                       _pid;                   ///< PID of the sandbox process
        tPluginType                 _pluginType;            ///< Type of the plugins managed by the sandbox
        tSandboxConfiguration       _configuration;         ///< Configuration
        tError                      _lastError;             ///< Last error that occured
        std::string                 _lastErrorDetails;      ///< Details associated with the last error that occured
        tStringList                 _plugins;               ///< List of the plugins loaded in the sandbox
        std::string                 _strLastLoadedPlugin;   ///< Last plugin loaded
        std::string                 _strLogFileSuffix;      ///< Suffix of the log files
        tLogFileInfosList           _logFilesInfos;         ///< List of the infos about the available log files
        ISandboxControllerListener* _pListener;             ///< Listener to use
        bool                        _bJailed;               ///< Indicates if the sandbox is in the 'jailed' state
    };
}

#endif
