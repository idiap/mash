#! /usr/bin/env python

################################################################################
# The MASH Framework contains the source code of all the servers in the
# "computation farm" of the MASH project (http://www.mash-project.eu),
# developed at the Idiap Research Institute (http://www.idiap.ch).
#
# Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
# Written by Philip Abbet (philip.abbet@idiap.ch)
#
# This file is part of the MASH Framework.
#
# The MASH Framework is free software: you can redistribute it and/or modify
# it under the terms of either the GNU General Public License version 2 or
# the GNU General Public License version 3 as published by the Free
# Software Foundation, whichever suits the most your needs.
#
# The MASH Framework is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public Licenses
# along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.
################################################################################


################################################################################
#                                                                              #
# Clustering Server                                                            #
#                                                                              #
# The Clustering Server is in charge of doing the link between the MASH        #
# platform and the clustring software developed at CNRS.                       #
#                                                                              #
################################################################################

from pymash import Server, ServerListener, OutStream, Message
import sys
import os
import subprocess
import shutil
import traceback
from optparse import OptionParser


################################## CONSTANTS ###################################

PROTOCOL = '1.0'


################################### GLOBALS ####################################

CONFIGURATION = None


############################### SERVER LISTENER ################################

#-------------------------------------------------------------------------------
# The listener that will handle the incoming connections
#-------------------------------------------------------------------------------
class ClusteringListener(ServerListener):

    # Class attributes
    handlers = {
        'STATUS': 'handleStatusCommand',
        'INFO': 'handleInfoCommand',
        'DONE': 'handleDoneCommand',
        'LOGS': 'handleLogsCommand',
        'ADD_SIGNATURE': 'handleAddSignatureCommand',
        'CLUSTER': 'handleClusterCommand',
    }
    
    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param socket     The socket on which the connection is established
    #---------------------------------------------------------------------------
    def __init__(self, socket, channel):
        super(ClusteringListener, self).__init__(socket, channel)
        self.outStream.open('ClusteringServer', os.path.join(CONFIGURATION.LOG_FOLDER, 'listener-$TIMESTAMP.log'))
    
    #---------------------------------------------------------------------------
    # Called when a command was received
    #
    # @param command    The command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleCommand(self, command):
        try:
            method = self.__getattribute__(ClusteringListener.handlers[command.name])
            return method(command.parameters)
        except:
            self.outStream.write(traceback.format_exc())
            if not(self.sendResponse(Message('UNKNOWN_COMMAND', [command.name]))):
                return ServerListener.ACTION_CLOSE_CONNECTION
            
            return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'STATUS' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleStatusCommand(self, arguments):
        if not(self.sendResponse('READY')):
            return ServerListener.ACTION_CLOSE_CONNECTION
            
        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'INFO' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleInfoCommand(self, arguments):
        if not(self.sendResponse(Message('TYPE', ['ClusteringServer']))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        if not(self.sendResponse(Message('PROTOCOL', [PROTOCOL]))):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'DONE' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleDoneCommand(self, arguments):
        self.sendResponse('GOODBYE')
        return ServerListener.ACTION_CLOSE_CONNECTION

    #---------------------------------------------------------------------------
    # Called when a 'LOGS' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleLogsCommand(self, arguments):
        content = self.outStream.dump(200 * 1024)
        if content is not None:
            self.sendResponse(Message('LOG_FILE', ['ClusteringServer.log', len(content)]))
            self.sendData(content)

        if not(self.sendResponse('END_LOGS')):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'ADD_SIGNATURE' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleAddSignatureCommand(self, arguments):

        # Check the arguments
        if len(arguments) != 2:
            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE
        
        filename = arguments[0]
        filesize = int(arguments[1])

        # Receive the file content
        content = self.waitData(filesize)
        if content is None:
            if not(self.sendResponse(Message('ERROR', ['Failed to receive the file']))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        # Save the file content
        if not(os.path.exists(os.path.abspath(CONFIGURATION.SIG_PATH))):
            os.makedirs(os.path.abspath(CONFIGURATION.SIG_PATH))

        f = open(os.path.join(os.path.abspath(CONFIGURATION.SIG_PATH), filename + '.signature'), 'wb')
        f.write(content)
        f.close()

        if not(self.sendResponse('OK')):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'CLUSTER' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleClusterCommand(self, arguments):

        # Create the configuration file
        f = open(os.path.join(os.path.abspath(CONFIGURATION.CLUSTERING_EXECUTABLE_FOLDER), 'config.txt'), 'w')
        f.write('SIG_PATH %s/\n' % os.path.abspath(CONFIGURATION.SIG_PATH))
        f.write('CACHE_PATH %s/\n' % os.path.abspath(CONFIGURATION.CACHE_PATH))
        f.write('OUT_PATH %s/\n' % os.path.abspath(CONFIGURATION.OUT_PATH))
        f.write('GEN_PATH %s/\n' % os.path.abspath(CONFIGURATION.GEN_PATH))
        f.write('TRIPLET_GENERATOR %s\n' % CONFIGURATION.TRIPLET_GENERATOR)
        f.close()

        # Setup the necessary folders
        current_dir = os.getcwd()

        if not(os.path.exists(os.path.abspath(CONFIGURATION.CACHE_PATH))):
            os.makedirs(os.path.abspath(CONFIGURATION.CACHE_PATH))

        if not(os.path.exists(os.path.abspath(CONFIGURATION.OUT_PATH))):
            os.makedirs(os.path.abspath(CONFIGURATION.OUT_PATH))
        
        os.chdir(CONFIGURATION.CLUSTERING_EXECUTABLE_FOLDER)

        # Perform the clustering
        try:
            p = subprocess.Popen(['./rvCluster', 'config.txt'],
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            ret = p.wait()
            if ret != 0:
                os.chdir(current_dir)

                self.outStream.write(p.stdout.read())
                if not(self.sendResponse(Message('ERROR', ['Failed to cluster the heuristics']))):
                    return ServerListener.ACTION_CLOSE_CONNECTION

                return ServerListener.ACTION_NONE
        except:
            os.chdir(current_dir)
            raise

        os.chdir(current_dir)

        # Send the results to the client
        f = open(os.path.join(os.path.abspath(CONFIGURATION.OUT_PATH), 'rv_inst.gexf'), 'rb')
        content = f.read()
        f.close()

        filesize = len(content)

        action = ServerListener.ACTION_NONE

        if not(self.sendResponse(Message('RESULTS', [filesize]))):
            action = ServerListener.ACTION_CLOSE_CONNECTION

        if (action == ServerListener.ACTION_NONE) and not(self.sendData(content)):
            return ServerListener.ACTION_CLOSE_CONNECTION

        shutil.rmtree(os.path.abspath(CONFIGURATION.SIG_PATH))
        shutil.rmtree(os.path.abspath(CONFIGURATION.OUT_PATH))

        return action


##################################### MAIN #####################################

if __name__ == "__main__":

    print """********************************************************************************
* Clustering Server
* Protocol: %s
********************************************************************************
""" % PROTOCOL

    # Setup of the command-line arguments parser
    usage = "Usage: %prog [options]"
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("--host", action="store", default="127.0.0.1", type="string",
                      dest="hostname", metavar="ADDRESS",
                      help="The host name or IP address that the server must listen on (default: 127.0.0.1)")
    parser.add_option("--port", action="store", default=13100, type="int",
                      dest="port", metavar="PORT",
                      help="The port that the server must listen on (default: 13100)")
    parser.add_option("--config", action="store", default="config", type="string",
                      dest="configurationFile", metavar="FILE", help="Path to the configuration file")
    parser.add_option("--info", action="store_true", default=False,
                      dest="info", metavar="FILE", help="Display the configuration and exit")
    parser.add_option("--verbose", action="store_true", default=False,
                      dest="verbose", help="Verbose mode")

    # Handling of the arguments
    (options, args) = parser.parse_args()

    OutStream.outputToConsole = options.verbose
    
    # Import the configuration
    path = os.path.dirname(os.path.abspath(options.configurationFile))
    if len(path) != 0:
        sys.path.append(path)

    module_name = os.path.basename(options.configurationFile)
    if module_name.endswith('.py'):
        module_name = module_name[:-3]

    CONFIGURATION = __import__(module_name)

    # Handle the '--info' option
    if options.info:
        for s in dir(CONFIGURATION):
            if not(s.startswith('__')):
                print "%s = %s" % (s, getattr(CONFIGURATION, s))
        sys.exit(0)
    
    sys.stdout.flush()
    
    # Listen for incoming connections
    server = Server(1, logFolder=CONFIGURATION.LOG_FOLDER)
    server.run(options.hostname, options.port, ClusteringListener)
