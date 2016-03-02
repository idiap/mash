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
# Compilation Server                                                           #
#                                                                              #
# The Compilation Server is in charge of the verification of each heuristic    #
# uploaded on the wesite. This includes:                                       #
#   - check that the heuristic can be compiled                                 #
#   - check that each required method of the heuristic is implemented          #
#   - check that the heuristic doesn't crash                                   #
#   - check that the heuristic doesn't timeout                                 #
#   - check that the heuristic doesn't perform a forbidden operation           #
#                                                                              #
################################################################################

from pymash import Server, ServerListener, OutStream, Message
import sys
import os
import subprocess
import signal
import traceback
from optparse import OptionParser


################################## CONSTANTS ###################################

PROTOCOL = '1.0'

FORBIDDEN_SYSTEM_CALLS = [
    'system',
    'popen',
    'pclose',
    'fdopen',
    'fopen',
    'freopen',
    'fclose',
    'tmpfile',
    'dlopen',
    'dlclose',
    'dlsym',
    'socket',
    'close',
    'execve',
    'execl',
    'execle',
    'execlp',
    'execv',
    'execvp',
    'execvP',
    'exit',
    '_Exit',
    '_exit',
    'atexit',
    'wait',
    'wait3',
    'wait4',
    'waitpid',
    'sleep',
    'nanosleep',
    'usleep',
    'fork',
    'vfork',
    'signal',
    'sigaction',
    'sigaltstack',
    'sigprocmask',
    'sigsuspend',
    'siginterrupt',
    'kill',
    'ptrace',
    '_longjmp',
    '_setjmp',
    'longjmp',
    'longjmperror',
    'setjmp',
    'siglongjmp',
    'sigsetjmp',
]


################################### GLOBALS ####################################

CONFIGURATION = None


############################### SERVER LISTENER ################################

#-------------------------------------------------------------------------------
# The listener that will handle the incoming connections
#-------------------------------------------------------------------------------
class CompilationListener(ServerListener):

    # Class attributes
    handlers = {
        'STATUS': 'handleStatusCommand',
        'INFO': 'handleInfoCommand',
        'DONE': 'handleDoneCommand',
        'LOGS': 'handleLogsCommand',
        'USE_HEURISTICS_REPOSITORY': 'handleUseHeuristicsRepositoryCommand',
        'CHECK_HEURISTIC': 'handleCheckHeuristicCommand',
    }
    
    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param socket     The socket on which the connection is established
    #---------------------------------------------------------------------------
    def __init__(self, socket, channel):
        super(CompilationListener, self).__init__(socket, channel)
        self.outStream.open('CompilationServer', os.path.join(CONFIGURATION.LOG_FOLDER, 'listener-$TIMESTAMP.log'))
    
    #---------------------------------------------------------------------------
    # Called when a command was received
    #
    # @param command    The command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleCommand(self, command):
        try:
            method = self.__getattribute__(CompilationListener.handlers[command.name])
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
        if not(self.sendResponse(Message('TYPE', ['CompilationServer']))):
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
            self.sendResponse(Message('LOG_FILE', ['CompilationServer.log', len(content)]))
            self.sendData(content)

        if not(self.sendResponse('END_LOGS')):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'USE_HEURISTICS_REPOSITORY' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleUseHeuristicsRepositoryCommand(self, arguments):

        # Check the arguments
        if len(arguments) != 1:
            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE
        
        repository_url = arguments[0]

        # Remove the previous cloned repository if any
        os.system('rm -rf %s' % CONFIGURATION.HEURISTICS_REPOSITORY_PATH)

        # Clone the repository
        p = subprocess.Popen(['git', 'clone', repository_url, CONFIGURATION.HEURISTICS_REPOSITORY_PATH],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        ret = p.wait()
        if ret != 0:
            self.outStream.write(p.stdout.read())
            if not(self.sendResponse(Message('ERROR', ['Failed to clone the repository']))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if not(self.sendResponse('OK')):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #---------------------------------------------------------------------------
    # Called when a 'CHECK_HEURISTIC' command was received
    #
    # @param arguments  Arguments of the command
    # @return           The action to perform
    #---------------------------------------------------------------------------
    def handleCheckHeuristicCommand(self, arguments):

        # Check the arguments
        if len(arguments) != 1:
            if not(self.sendResponse(Message('INVALID_ARGUMENTS', arguments))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE
        
        # Determine the path of the source file in the repository
        heuristic_fullname = arguments[0]

        if heuristic_fullname.endswith('/1'):
            heuristic_fullname = heuristic_fullname[:-2]

        parts = heuristic_fullname.split('/')
        
        username = parts[0].lower()
        heuristic_name = parts[1].lower()

        if len(parts) == 3:
            filename = '%s_v%s.cpp' % (heuristic_name, parts[2])
        else:
            filename = '%s.cpp' % heuristic_name

        fullpath = os.path.join(CONFIGURATION.HEURISTICS_REPOSITORY_PATH, username, filename)
        if not(os.path.exists(fullpath)):
            if not(self.sendResponse(Message('ERROR', ['Heuristic not found']))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        # Compile the heuristic
        (result, text) = self.compile(heuristic_fullname, fullpath)
        if not(result):
            os.system('rm -rf %s' % CONFIGURATION.HEURISTICS_BUILD_FOLDER)
            
            if not(self.sendResponse(Message('COMPILATION_ERROR', [text]))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE
        
        if not(self.sendResponse('COMPILATION_OK')):
            os.system('rm -rf %s' % CONFIGURATION.HEURISTICS_BUILD_FOLDER)
            return ServerListener.ACTION_CLOSE_CONNECTION

        # Analyze the compiled dynamic library
        (result, text) = self.analyze(username, filename)
        if not(result):
            os.system('rm -rf %s' % CONFIGURATION.HEURISTICS_BUILD_FOLDER)

            if not(self.sendResponse(Message('ANALYZE_ERROR', [text]))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE
        
        if not(self.sendResponse('ANALYZE_OK')):
            os.system('rm -rf %s' % CONFIGURATION.HEURISTICS_BUILD_FOLDER)
            return ServerListener.ACTION_CLOSE_CONNECTION

        # Test the heuristic
        (result, error, description, context, stacktrace) = self.test(heuristic_fullname)

        os.system('rm -rf %s' % CONFIGURATION.HEURISTICS_BUILD_FOLDER)

        if not(result):
            if (error is not None) and not(self.sendResponse(error)):
                return ServerListener.ACTION_CLOSE_CONNECTION

            if (description is not None) and not(self.sendResponse(Message('TEST_ERROR', [description]))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            if (context is not None) and not(self.sendResponse(Message('CONTEXT', [context]))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            if error == 'HEURISTIC_CRASH':
                if stacktrace is not None:
                    if not(self.sendResponse(Message('STACKTRACE', [stacktrace]))):
                        return ServerListener.ACTION_CLOSE_CONNECTION
                else:
                    if not(self.sendResponse('NO_STACKTRACE')):
                        return ServerListener.ACTION_CLOSE_CONNECTION

            return ServerListener.ACTION_NONE

        if not(self.sendResponse('TEST_OK')):
            return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE

    #-------------------------------------------------------------------------------
    # Compile a heuristic
    #-------------------------------------------------------------------------------
    def compile(self, heuristic_name, filename):

        def _filterPaths(text, to_remove):
            for path in to_remove:
                if not(path.endswith(os.path.sep)):
                    path = path + os.path.sep
                text = text.replace(path, '')
            return text

        # Check that the source file exists
        filename = os.path.abspath(filename)
        if not(os.path.exists(filename)):
            sys.exit(1)

        bin_path = os.path.abspath(CONFIGURATION.BIN_PATH)

        # Remove the build dir if one exists
        if os.path.exists(CONFIGURATION.HEURISTICS_BUILD_FOLDER):
            os.system('rm -rf %s' % CONFIGURATION.HEURISTICS_BUILD_FOLDER)

        # Create the build dir
        current_dir = os.getcwd()
        os.makedirs(CONFIGURATION.HEURISTICS_BUILD_FOLDER)
        os.chdir(CONFIGURATION.HEURISTICS_BUILD_FOLDER)

        text = None

        # Generate the makefiles
        p = subprocess.Popen(['cmake', '-DSRC_FILE=%s' % filename,
                              '-DFULL_HEURISTIC_NAME=%s' % heuristic_name,
                              '-DHEURISTICS_BUILD_DIR=%s' % os.path.join(bin_path, CONFIGURATION.HEURISTICS_DEST_FOLDER),
                              '-DMASH_BIN_DIR=%s' % bin_path,
                              os.path.join(current_dir, CONFIGURATION.HEURISTICS_CMAKE_PATH)],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        ret = p.wait()
        if ret == 0:
            # Compile the heuristic
            p = subprocess.Popen('make', stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            ret = p.wait()
            if (ret != 0):
                text = _filterPaths(p.stdout.read(), [os.path.join(current_dir, CONFIGURATION.HEURISTICS_REPOSITORY_PATH),
                                                      os.path.join(current_dir, CONFIGURATION.HEURISTICS_CMAKE_PATH)])
        else:
            text = 'Failed to generate the makefiles\n\n' + p.stdout.read()

        os.chdir(current_dir)

        return ((ret == 0), text)

    #-------------------------------------------------------------------------------
    # Analyze a dynamic library containing a heuristic to ensure that certain
    # forbidden system functions aren't used
    #-------------------------------------------------------------------------------
    def analyze(self, username, filename):

        # Determine the name of the dynamic library containing the heuristic
        dynlib = 'lib' + os.path.splitext(filename)[0] + '.so'
        fullpath = os.path.join(CONFIGURATION.HEURISTICS_BIN_PATH, username, dynlib)

        # Search for forbidden system calls
        for function in FORBIDDEN_SYSTEM_CALLS:
            p = subprocess.Popen('nm %s | grep "U _%s"' % (fullpath, function),
                                 shell=True, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
            p.wait()

            if p.returncode != 0:
                p = subprocess.Popen('nm %s | grep "U %s"' % (fullpath, function),
                                     shell=True, stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)
                p.wait()

            if p.returncode == 0:
                return (False, "Forbidden use of the '%s()' function" % function)

            details = p.stderr.read()
            if len(details) > 0:
                return (False, details)

        return (True, None)

    #-------------------------------------------------------------------------------
    # Run a heuristic to check that it doesn't crash, never return, eat too much
    # memory, break the rules, ...
    #-------------------------------------------------------------------------------
    def test(self, heuristic_name):

        class TimeExceededError(Exception):
            pass

        def alarmHandler(signum, frame):
            raise TimeExceededError


        # Go into the binary folder
        current_dir = os.getcwd()
        os.chdir(CONFIGURATION.BIN_PATH)

#        images = ['car.ppm', 'bear.jpg', 'cup.ppm', 'dolphin.jpg', 'duck.ppm', 'monkey.jpg']
        images = ['car.ppm', 'bear.jpg', 'cup.ppm', 'duck.ppm', 'monkey.jpg']
        for test_image in images:

            # Test the heuristic (note: stderr is used by 'checkheuristic' to indicates why the test failed)
            p = subprocess.Popen(['./checkheuristic',
                                  '--heuristicsdir=%s' % CONFIGURATION.HEURISTICS_DEST_FOLDER,
                                  '--datadir=%s' % CONFIGURATION.DATA_PATH,
                                  '--logdir=%s' % CONFIGURATION.LOG_FOLDER,
                                  '--coredump-template=%s' % CONFIGURATION.CORE_DUMP_TEMPLATE,
                                  '--sandbox-username=%s' % CONFIGURATION.SANDBOX_USERNAME,
                                  '--sandbox-jaildir=%s' % CONFIGURATION.SANDBOX_JAILDIR,
                                  '--sandbox-scriptsdir=%s' % CONFIGURATION.SANDBOX_SCRIPTSDIR,
                                  '--sandbox-tempdir=%s' % CONFIGURATION.SANDBOX_TEMPDIR,
                                  '--source-heuristics=%s' % os.path.join(current_dir, CONFIGURATION.HEURISTICS_REPOSITORY_PATH),
                                  heuristic_name,
                                  test_image,
                                 ],
                                 stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, universal_newlines=True)

            (output, errors) = p.communicate()
            if p.returncode != 0:
                os.chdir(current_dir)
                
                error       = None
                description = None
                context     = None
                stacktrace  = None
                
                print errors
                
                if errors.startswith('HEURISTIC_CRASH'):
                    error = 'HEURISTIC_CRASH'
                elif errors.startswith('HEURISTIC_TIMEOUT'):
                    error = 'HEURISTIC_TIMEOUT'
                elif errors.startswith('HEURISTIC_ERROR'):
                    start_offset = errors.find('<mash:error>')
                    start_offset += 12
                    end_offset = errors.find('</mash:error>', start_offset)
                    description = errors[start_offset:end_offset]
                else:
                    description = errors
                    
                start_offset = errors.find('<mash:context>')
                if start_offset > 0:
                    start_offset += 14
                    end_offset = errors.find('</mash:context>', start_offset)
                    context = errors[start_offset:end_offset]

                start_offset = errors.find('<mash:stacktrace>')
                if start_offset > 0:
                    start_offset += 17
                    end_offset = errors.find('</mash:stacktrace>', start_offset)
                    stacktrace = errors[start_offset:end_offset]

                return (False, error, description, context, stacktrace)

        os.chdir(current_dir)

        return (True, None, None, None, None)


################################### FUNCTIONS ##################################

def processSetting(value, instance):
    if instance != '':
        return value.replace('$INSTANCE', instance)
    else:
        value = value.replace('$INSTANCE/', '')
        value = value.replace('/$INSTANCE', '')
        value = value.replace('-$INSTANCE', '')
        value = value.replace('$INSTANCE', '')
        return value


##################################### MAIN #####################################

if __name__ == "__main__":

    print """********************************************************************************
* Compilation Server
* Protocol: %s
********************************************************************************
""" % PROTOCOL

    # Setup of the command-line arguments parser
    usage = "Usage: %prog [options]"
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("--host", action="store", default="127.0.0.1", type="string",
                      dest="hostname", metavar="ADDRESS",
                      help="The host name or IP address that the server must listen on (default: 127.0.0.1)")
    parser.add_option("--port", action="store", default=13000, type="int",
                      dest="port", metavar="PORT",
                      help="The port that the server must listen on (default: 13000)")
    parser.add_option("--instance", action="store", default="", type="string",
                      dest="instance", metavar="INSTANCE",
                      help="The value that must be used to replace $INSTANCE in the configuration file (default: '')")
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

    CONFIGURATION.HEURISTICS_REPOSITORY_PATH    = processSetting(CONFIGURATION.HEURISTICS_REPOSITORY_PATH, options.instance)
    CONFIGURATION.HEURISTICS_CMAKE_PATH         = processSetting(CONFIGURATION.HEURISTICS_CMAKE_PATH, options.instance)
    CONFIGURATION.HEURISTICS_DEST_FOLDER        = processSetting(CONFIGURATION.HEURISTICS_DEST_FOLDER, options.instance)
    CONFIGURATION.BIN_PATH                      = processSetting(CONFIGURATION.BIN_PATH, options.instance)
    CONFIGURATION.HEURISTICS_BUILD_FOLDER       = processSetting(CONFIGURATION.HEURISTICS_BUILD_FOLDER, options.instance)
    CONFIGURATION.DATA_PATH                     = processSetting(CONFIGURATION.DATA_PATH, options.instance)
    CONFIGURATION.LOG_FOLDER                    = processSetting(CONFIGURATION.LOG_FOLDER, options.instance)
    CONFIGURATION.CORE_DUMP_TEMPLATE            = processSetting(CONFIGURATION.CORE_DUMP_TEMPLATE, options.instance)
    CONFIGURATION.SANDBOX_USERNAME              = processSetting(CONFIGURATION.SANDBOX_USERNAME, options.instance)
    CONFIGURATION.SANDBOX_JAILDIR               = processSetting(CONFIGURATION.SANDBOX_JAILDIR, options.instance)
    CONFIGURATION.SANDBOX_SCRIPTSDIR            = processSetting(CONFIGURATION.SANDBOX_SCRIPTSDIR, options.instance)
    CONFIGURATION.SANDBOX_TEMPDIR               = processSetting(CONFIGURATION.SANDBOX_TEMPDIR, options.instance)

    CONFIGURATION.HEURISTICS_BIN_PATH = os.path.join(CONFIGURATION.BIN_PATH, CONFIGURATION.HEURISTICS_DEST_FOLDER)

    # Handle the '--info' option
    if options.info:
        for s in dir(CONFIGURATION):
            if not(s.startswith('__')):
                print "%s = %s" % (s, getattr(CONFIGURATION, s))
        sys.exit(0)
    
    sys.stdout.flush()
    
    # Listen for incoming connections
    server = Server(1, logFolder=CONFIGURATION.LOG_FOLDER)
    server.run(options.hostname, options.port, CompilationListener)
