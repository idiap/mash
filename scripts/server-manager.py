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
# Server Manager                                                               #
#                                                                              #
# Keeps several instances of the MASH servers (Compilation, Experiment,        #
# Application) running.                                                        #
#                                                                              #
################################################################################

import sys
import os
import traceback
import subprocess
import select
import datetime
import signal
import signal
from optparse import OptionParser
from threading import Thread

sys.path.append(os.getcwd())

from pymash import ThreadedServer
from pymash import ServerListener
from pymash import Client
from pymash import OutStream
from pymash import CommunicationChannel
from pymash import Message


################################### GLOBALS ####################################

CONFIGURATION = None


################################## FUNCTIONS ###################################

def terminate_signal_handler(signum, frame):
    raise KeyboardInterrupt()


################################ COMMAND: deamon ###############################

#-------------------------------------------------------------------------------
# The listener that will handle the incoming connections
#-------------------------------------------------------------------------------
class DeamonListener(ServerListener):
    
    deamon = None
    
    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param socket     The socket on which the connection is established
    #---------------------------------------------------------------------------
    def __init__(self, socket):
        super(DeamonListener, self).__init__(socket)
    
    #---------------------------------------------------------------------------
    # Called when a command was received
    #
    # @param command    The command
    # @return           'False' if the connection must be closed
    #---------------------------------------------------------------------------
    def handleCommand(self, command):
        try:
            DeamonListener.deamon.handleCommand(command)

            if not(self.sendResponse('OK')):
                return ServerListener.ACTION_CLOSE_CONNECTION
            
            return ServerListener.ACTION_NONE
        except:
            if not(self.sendResponse(Message('ERROR', [traceback.format_exc()]))):
                return ServerListener.ACTION_CLOSE_CONNECTION
            
            return ServerListener.ACTION_NONE


#-------------------------------------------------------------------------------
# Used to retrieve the configuration of a server instance
#-------------------------------------------------------------------------------
class ServerInfos:
    
    def __init__(self, server_infos):
        self.infos = {}

        # Retrieve the server type infos
        server_types = filter(lambda x: server_infos['name'].startswith(x), CONFIGURATION.server_types.keys())
        if len(server_types) != 0:
            for (name, value) in CONFIGURATION.server_types[server_types[0]].items():
                self.infos[name] = value

        for (name, value) in server_infos.items():
            self.infos[name] = value

        for (name, value) in filter(lambda x: str(x[1]).find('$(') >= 0, self.infos.items()):
            start = value.find('$(')
            end = value.find(')', start)
            
            if end == -1:
                continue

            self.infos[name] = value[0:start] + self.infos[value[start+2:end]] + value[end+1:]
        
        if not(os.path.isabs(self.infos['cwd'])):
            self.infos['cwd'] = os.path.join(os.getcwd(), self.infos['cwd'])

    def __getitem__(self, name):
        if self.infos.has_key(name):
            return self.infos[name]
        
        return None

    def commandLine(self):
        command_line = []
        
        if self['user'] is not None:
            command_line.extend(['su', '-', '-m', self['user'], '-c'])
                
            if self['path'].find(' ') >= 0:
                args = '"%s"' % self['path']
            else:
                args = self['path']

            for (option, value) in filter(lambda x: x[0].startswith('--'), self.infos.items()):
                if (isinstance(value, str) or isinstance(value, unicode)) and value.find(' ') >= 0:
                    args += ' %s="%s"' % (option, value)
                else:
                    args += ' %s=%s' % (option, str(value))

            command_line.append(args)

        else:
            command_line.append(self['path'])

            for (option, value) in filter(lambda x: x[0].startswith('--'), self.infos.items()):
                if (isinstance(value, str) or isinstance(value, unicode)) and (value.find(' ') >= 0):
                    command_line.append('%s="%s"' % (option, value))
                else:
                    command_line.append('%s=%s' % (option, str(value)))
        
        return command_line


#-------------------------------------------------------------------------------
# The deamon that manage all the servers
#-------------------------------------------------------------------------------
class Deamon:

    #---------------------------------------------------------------------------
    # Constructor
    #---------------------------------------------------------------------------
    def __init__(self):
        self.server = None
        (self.channel_write, self.channel_read) = CommunicationChannel.create(CommunicationChannel.CHANNEL_TYPE_SIMPLEX)
        self.server_instances = {}

        DeamonListener.deamon = self

        self.outStream = OutStream()
        self.outStream.open('Deamon', os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), 'logs/deamon-$TIMESTAMP.log'))

    #---------------------------------------------------------------------------
    # Start the deamon
    #---------------------------------------------------------------------------
    def start(self):

        global CONFIGURATION

        # Start the server
        self.server = ThreadedServer(CONFIGURATION.host, CONFIGURATION.port, DeamonListener,
                                     name='DeamonServer',
                                     logFolder=os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), 'logs'))
        self.server.start()

        signal.signal(signal.SIGTERM, terminate_signal_handler)

        try:
            main_buffer = ''
            start_time = datetime.datetime.now()

            # Starts the server instances
            for server_infos in CONFIGURATION.servers:
                infos = ServerInfos(server_infos)
                
                self.outStream.write('Starting %s...\n' % infos['name'])
                if self.server_instances.has_key(infos['name']):
                    self.outStream.write('ERROR: Another server already use this name\n')
                    continue

                try:
                    self.server_instances[infos['name']] = subprocess.Popen(infos.commandLine(),
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True,
                            cwd=infos['cwd'])
                except:
                    self.outStream.write("ERROR: Failed to start the server\n")
                    self.outStream.write("Details:\n")
                    self.outStream.write(traceback.format_exc())
                    
                    if self.server_instances.has_key(infos['name']):
                        self.server_instances.pop(infos['name'])
            
            # Build the list of handles we must listen to
            select_list = [self.channel_read.readPipe]
            select_list.extend(map(lambda x: x.stdout, self.server_instances.values()))
            
            while (True):
                current_time = datetime.datetime.now()
                delta = datetime.timedelta(days=1)
                if current_time > start_time + delta:
                    start_time = current_time
                    self.outStream.write('----------------------------------------------------------------\n')
                    self.outStream.write('Opening a new log file...\n')
                    self.outStream.close()
                    self.outStream = OutStream()
                    self.outStream.open('Deamon', os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), 'logs/deamon-$TIMESTAMP.log'))
                else:
                    delta = (start_time + delta) - current_time

                timeout = delta.days * 24 * 60 * 60 + delta.seconds + 1

                # Wait for a message from a client or for the crash of a server instance
                self.outStream.write('----------------------------------------------------------------\n')
                self.outStream.write('Waiting...\n')
                ready_to_read, ready_to_write, in_error = select.select(select_list, [], select_list, timeout)

                # Command sent by a client
                if self.channel_read.readPipe in ready_to_read:
                    command = self.channel_read.waitMessage()
                    
                    self.outStream.write('Got command: %s\n' % command.toString())
                    
                    if command.name == 'RELOAD':
                        CONFIGURATION = reload(CONFIGURATION)
                        
                    elif command.name == 'START':
                        server_list = filter(lambda x: x['name'] == command.parameters[0], CONFIGURATION.servers)
                        if len(server_list) == 1:
                            infos = ServerInfos(server_list[0])
                            self.outStream.write("Starting the server '%s'...\n" % infos['name'])

                            if self.server_instances.has_key(infos['name']):
                                self.outStream.write('ERROR: Another server already use this name\n')
                            else:
                                try:
                                    self.server_instances[infos['name']] = subprocess.Popen(infos.commandLine(),
                                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True,
                                            cwd=infos['cwd'])
                                except:
                                    self.outStream.write("ERROR: Failed to start the server\n")
                                    self.outStream.write("Details:\n")
                                    self.outStream.write(traceback.format_exc())
                                    
                                    if self.server_instances.has_key(infos['name']):
                                        self.server_instances.pop(infos['name'])

                                if self.server_instances.has_key(infos['name']):
                                    select_list.append(self.server_instances[infos['name']].stdout)

                        elif len(server_list) == 0:
                            self.outStream.write("Can't start the server: not found in the configuration\n")
                        else:
                            self.outStream.write("Can't start the server: There are several servers with this name found in the configuration\n")

                    elif command.name == 'STOP':
                        if self.server_instances.has_key(command.parameters[0]):
                            self.outStream.write("Stopping the server '%s'...\n" % command.parameters[0])
                            os.kill(self.server_instances[command.parameters[0]].pid, signal.SIGTERM)
                            os.waitpid(self.server_instances[command.parameters[0]].pid, 0)
                            select_list.remove(self.server_instances[command.parameters[0]].stdout)
                            self.server_instances.pop(command.parameters[0])
                            self.outStream.write("Server stopped\n")
                        else:
                            self.outStream.write("Can't stop the server: no instance running\n")

                # A server instance crashed
                for (name, instance) in filter(lambda x: (x[1] is not None) and (x[1].poll() is not None), self.server_instances.items()):
                    self.outStream.write('----------------------------------------------------------------\n')
                    self.outStream.write("The server '%s' crashed!\n" % name)
                    self.outStream.write("STDOUT:\n%s\n" % instance.stdout.read())

                    server_list = filter(lambda x: x['name'] == name, CONFIGURATION.servers)
                    if len(server_list) == 1:
                        self.outStream.write("Restarting it...\n")
                        infos = ServerInfos(server_list[0])

                        select_list.remove(self.server_instances[name].stdout)
                        self.server_instances.pop(name)

                        try:
                            self.server_instances[name] = subprocess.Popen(infos.commandLine(),
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True,
                                    cwd=infos['cwd'])
                        except:
                            self.outStream.write("ERROR: Failed to restart the server\n")
                            self.outStream.write("Details:\n")
                            self.outStream.write(traceback.format_exc())
                            
                            if self.server_instances.has_key(name):
                                self.server_instances.pop(name)

                        if self.server_instances.has_key(name):
                            select_list.append(self.server_instances[name].stdout)
                            
                    elif len(server_list) == 0:
                        self.outStream.write("Can't restart it: no more in the configuration\n")
                    else:
                        self.outStream.write("Can't restart it: There are several servers with this name found in the configuration\n")
                
                # A server instance wrote something on its standard output/error
                to_read = filter(lambda x: x in select_list, ready_to_read)
                for stream in to_read:
                    servers = filter(lambda x: x[1].stdout is stream, self.server_instances.items())
                    if len(servers) == 1:
                        self.outStream.write("Consuming standard output of server '%s'...\n" % servers[0][0])

                    while True:
                        r, w, e = select.select([stream], [], [], 0)
                        if stream in r:
                            stream.readline()
                        else:
                            break

        except KeyboardInterrupt:
            pass

        self.outStream.write("Stop to listen to commands...\n")
        self.server.stop()
        self.channel_write.close()
        self.channel_read.close()

        for (name, instance) in self.server_instances.items():
            self.outStream.write("Stopping the server '%s'...\n" % name)
            os.kill(instance.pid, signal.SIGTERM)
            os.waitpid(instance.pid, 0)

    #---------------------------------------------------------------------------
    # Ask the the deamon to execute a command
    #
    # @remark   Thread-safe
    #---------------------------------------------------------------------------
    def handleCommand(self, command):
        self.channel_write.sendMessage(command)


#---------------------------------------------------------------------------
# Starts the deamon
#---------------------------------------------------------------------------
def execute_deamon():
    deamon = Deamon()
    deamon.start()



################################ OTHER COMMANDS ################################

def send_command(command):
    client = Client()
    
    if not(client.connect(CONFIGURATION.host, CONFIGURATION.port)):
        print "Failed to connect to the Server Manager Deamon"
        sys.exit(-1)

    if not(client.sendCommand(command)):
        print "Failed to send the command to the server"
        sys.exit(-1)

    response = client.waitResponse()
    if response is None:
        print "Failed to wait for a response from the server"
        sys.exit(-1)

    if response.name != 'OK':
        print "%s" % response.toString()
        sys.exit(-1)

    client.close()
    
    print "Command successfully sent to the deamon"


def execute_reload():
    send_command('RELOAD')


def execute_start(server_name):
    send_command(Message('START', [server_name]))


def execute_stop(server_name):
    send_command(Message('STOP', [server_name]))


def execute_status():
    result = []
    width = 0
    for server_infos in CONFIGURATION.servers:
        infos = ServerInfos(server_infos)
        
        if infos['--host'] and infos['--port']:
            result.append((infos['name'], infos['--host'], infos['--port']))
        elif infos['#host'] and infos['#port']:
            result.append((infos['name'], infos['#host'], infos['#port']))

        width = max(len(infos['name']), width)

    diff = (width + 2 - 6) / 2
    diff2 = width + 2 - (diff + 6)

    print
    print ' ' * diff + 'SERVER' + ' ' * diff2 + 'STATUS'
    print '-' * (width + 9)

    for (name, address, port) in result:
        client = Client()

        if not(client.connect(address, port)):
            print ('%-' + str(width + 2) + 'sOFFLINE') % name
            continue

        print ('%-' + str(width + 2) + 'sONLINE') % name

        client.close()

    print
    

def execute_list():
    result = []
    width_name = 0
    width_cwd = 0
    for server_infos in CONFIGURATION.servers:
        infos = ServerInfos(server_infos)
        result.append((infos['name'], infos['cwd'], infos.commandLine()))
        width_name = max(len(infos['name']), width_name)
        width_cwd = max(len(infos['cwd']), width_cwd)
    
    width_name += 2
    width_cwd += 4
    
    spaces_before_name = (width_name - 6) / 2
    pos_after_name = spaces_before_name + 6
    spaces_after_name = width_name - pos_after_name

    spaces_before_cwd = (width_cwd - 3) / 2
    pos_after_cwd = spaces_before_cwd + 3
    spaces_after_cwd = width_cwd - pos_after_cwd
        
    print
    print ' ' * spaces_before_name + 'SERVER' + ' ' * (spaces_after_name + spaces_before_cwd) + 'CWD' + ' ' * (spaces_after_cwd + 20) + 'COMMAND LINE'
    print '-' * (width_name + width_cwd + 80)
    
    for (name, cwd, command_line) in result:
        print ('%-' + str(width_name) + 's  %-' + str(width_cwd-2) + 's  %s') % (name, cwd, ' '.join(command_line))

    print


##################################### MAIN #####################################

if __name__ == "__main__":

    # Setup of the command-line arguments parser
    usage = """Usage: %prog [args] deamon
            Starts an instance of each server found in the configuration file

       %prog [args] start server-name
            Starts the specified server
            
       %prog [args] stop server-name
            Stops the specified server
       
       %prog [args] reload
            Reload the list of servers from the configuration file

       %prog [args] status
            Display the status of each server found in the configuration file

       %prog [args] list
            List the servers (and their command-lines) found in the configuration file
"""
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("--config", action="store", default="config", type="string",
                      dest="configurationFile", metavar="FILE", help="Path to the configuration file (without extension)")
    parser.add_option("--verbose", action="store_true", default=False,
                      dest="verbose", metavar="FILE", help="Verbose mode")

    # Handling of the arguments
    (options, args) = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        exit(-1)

    OutStream.outputToConsole = options.verbose

    # Import the configuration
    path = os.path.dirname(os.path.abspath(options.configurationFile))
    if len(path) != 0:
        sys.path.append(path)

    module_name = os.path.basename(options.configurationFile)
    if module_name.endswith('.py'):
        module_name = module_name[:-3]

    CONFIGURATION = __import__(module_name)
    
    # Execute the command
    if args[0] == 'deamon':
        execute_deamon()
    elif args[0] == 'reload':
        execute_reload()
    elif args[0] == 'status':
        execute_status()
    elif args[0] == 'list':
        execute_list()
    elif len(args) != 2:
        parser.print_help()
        exit(-1)
    elif args[0] == 'start':
        execute_start(args[1])
    elif args[0] == 'stop':
        execute_stop(args[1])
    else:
        parser.print_help()
        exit(-1)
