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
#
# This script is used to test the implementation of the Experiment Server
#
################################################################################


import sys
import os
import subprocess
import time
import signal
from optparse import OptionParser


#################################### GLOBALS ###################################

CONFIGURATION       = None
compilation_server  = None


################################## FUNCTIONS ###################################

def write(text):
    if not(CONFIGURATION.quiet):
        print text

def error(text):
    print 'ERROR: %s' % text
    if compilation_server is not None:
        os.kill(compilation_server.pid, signal.SIGTERM)
    sys.exit(1)


##################################### MAIN #####################################

if __name__ == "__main__":

    # Setup of the command-line arguments parser
    usage = "Usage: %prog { compilation | crash | timeout | error | none } HEURISTIC_NAME [METHOD_NAME]"
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("-q", "--quiet", action="store_true", default=False,
                      dest="quiet", help="Don't write non-error messages to the console output")

    # Handling of the arguments
    (CONFIGURATION, args) = parser.parse_args()
    if len(args) > 3:
        parser.print_help()
        sys.exit(1)

    script_dir = os.path.abspath(os.path.dirname(sys.argv[0]))

    test_type              = args[0]
    heuristic_name         = args[1]
    method_name            = None
    
    if test_type not in ['compilation', 'none']:
        method_name = args[2]

    # Start the Compilation Server
    os.chdir('%s/../../compilation-server/' % script_dir)

    command = "./compilation-server.py --port=13010 --config=%s/compilation-server-config.py --instance=tests_compilation_server" % script_dir
    compilation_server = subprocess.Popen(command.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    time.sleep(2)
    
    if compilation_server.poll():
        output = compilation_server.stdout.read()
        compilation_server = None
        error('Failed to start the compilation server, output: \n' + output)

    # Establish a connection with the Compilation Server
    sys.path.append('%s/../../' % script_dir)

    from pymash import Client
    from pymash import Message

    client = Client()
    if not(client.connect('127.0.0.1', 13010)):
        error('Failed to connect to the server')

    # Start the test of the heuristic
    if not(client.sendCommand(Message('CHECK_HEURISTIC', [heuristic_name]))):
        error("Failed to send command 'CHECK_HEURISTIC' to the server")

    response = client.waitResponse()
    if response is None:
        error("Failed to wait for response to command 'CHECK_HEURISTIC' from the server")

    if test_type == 'compilation':
        if response.name == 'COMPILATION_OK':
            error("Successfully compiled the heuristic: %s" % response.toString())

        if response.name != 'COMPILATION_ERROR':
            error("Failed to report the compilation error: %s" % response.toString())

    else:
        if response.name != 'COMPILATION_OK':
            error("Failed to compile the heuristic: %s" % response.toString())

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response to command 'CHECK_HEURISTIC' from the server")

        if response.name != 'ANALYZE_OK':
            error("Failed to analyze the heuristic: %s" % response.toString())

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response to command 'CHECK_HEURISTIC' from the server")

    print response.toString()
    
    # Check that the Experiment Server reported the error
    if test_type == 'crash':
        if response.name != 'HEURISTIC_CRASH':
            error("No crash reported")

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for the context of the crash from the server")

        print response.toString()

        if response.name != 'CONTEXT':
            error("No context reported")

        if not(response.parameters[0].lower().startswith('method: %s\n' % method_name)):
            error("The method name isn't reported in the context")

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for the stacktrace from the server")

        print response.toString()

        if response.name != 'STACKTRACE':
            error("No stacktrace reported")

    elif test_type == 'timeout':
        if response.name != 'HEURISTIC_TIMEOUT':
            error("No crash reported")

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for the context of the crash from the server")

        print response.toString()

        if response.name != 'CONTEXT':
            error("No context reported")

        if not(response.parameters[0].lower().startswith('method: %s\n' % method_name)):
            error("The method name isn't reported in the context")

    elif test_type == 'error':
        if response.name != 'TEST_ERROR':
            error("No error reported")

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for the context of the crash from the server")

        print response.toString()

        if response.name != 'CONTEXT':
            error("No context reported")

    elif test_type == 'compilation':
        pass

    else:
        if response.name != 'TEST_OK':
            error("Error reported")


    # Stop the Compilation Server
    client.close()
    
    if compilation_server is not None:
        os.kill(compilation_server.pid, signal.SIGTERM)

    write('Done')
