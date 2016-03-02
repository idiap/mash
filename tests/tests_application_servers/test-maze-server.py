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
# This script is used to test the implementation of the Maze Server
#
################################################################################


import sys
import os
import subprocess
import time
import signal
from optparse import OptionParser

# Delayed modules
pymash = None


#################################### GLOBALS ###################################

CONFIGURATION = None
client = None
server = None


################################## FUNCTIONS ###################################

def output(text):
    if not(CONFIGURATION.quiet):
        print text

def error(text):
    print 'ERROR: %s' % text
    if client is not None:
        client.close()
    if server is not None:
        os.kill(server.pid, signal.SIGTERM)
    sys.exit(1)

def CHECK_EQUAL(expected, actual):
    if expected != actual:
        error("Expected %s, got %s" % (str(expected), str(actual)))

def check_request(request, expected_responses):
    if not(client.sendCommand(pymash.Message(request[0], request[1]))):
        error("Failed to send the command '%s' to the server" % request[0])

    for expected_response, expected_args in expected_responses:
        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response '%s' from the server" % expected_response)

        CHECK_EQUAL(expected_response, response.name)

        if len(expected_args) > 0:
            CHECK_EQUAL(len(expected_args), len(response.parameters))
        elif not(response.parameters is None):
            error("Got parameters when expecting none: %s" % str(response.parameters))
        
        for i in range(0, len(expected_args)):
            if isinstance(expected_args[i], float):
                CHECK_EQUAL(expected_args[i], float(response.parameters[i]))
            elif isinstance(expected_args[i], int):
                CHECK_EQUAL(expected_args[i], int(response.parameters[i]))
            else:
                CHECK_EQUAL(str(expected_args[i]), str(response.parameters[i]))


##################################### MAIN #####################################

if __name__ == "__main__":

    # Setup of the command-line arguments parser
    usage = """Usage: %prog --server=PATH [options] PORT
       %prog [options] ADDRESS PORT

If the --server option is used, an application server is started on the local
machine, using the given port. Otherwise the program will attempt to connect to
the specified address."""
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("-q", "--quiet", action="store_true", default=False,
                      dest="quiet", help="Don't write non-error messages to the console output")
    parser.add_option("--pymash", action="store", default="pymash", type="string",
                      dest="pymash_path", help="Path to the pymash module")
    parser.add_option("--server", action="store", default=None, type="string", metavar="PATH",
                      dest="server_path", help="Path to the application server to execute")

    # Handling of the arguments
    (CONFIGURATION, args) = parser.parse_args()
    if (CONFIGURATION.server_path is None) and (len(args) == 0):
        parser.print_help()
        sys.exit(0)

    # Importation of the pymash module
    path = os.path.dirname(CONFIGURATION.pymash_path)
    module_name = os.path.basename(CONFIGURATION.pymash_path)
    if len(path) != 0:
        sys.path.append(path)
    pymash = __import__(module_name)

    # Start the server application if needed
    if CONFIGURATION.server_path is not None:
        command = "%s --host=127.0.0.1 --port=%d" % (os.path.abspath(CONFIGURATION.server_path), int(args[0]))
        server = subprocess.Popen(command.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=os.path.dirname(CONFIGURATION.server_path))
        time.sleep(1)

        if server.poll():
            output = server.stdout.read()
            server = None
            error('Failed to start the application server, output: \n' + output)

        server_address = '127.0.0.1'
        server_port = int(args[0])
    else:
        server_address = args[0]
        server_port = int(args[1])
        
    # Create a client and connect to the application server
    client = pymash.Client()
    if not(client.connect(server_address, server_port)):
        error('Failed to connect to the application server')
    
    # Test that the protocol is respected
    output('TEST: Info retrieval')

    check_request(('STATUS', []), [('READY', [])])
    check_request(('INFO', []), [('TYPE', ['ApplicationServer']),
                                 ('SUBTYPE', ['Interactive']),
                                 ('PROTOCOL', ['1.4'])
                                ])

                                 
    output('TEST: Goals listing')

    if not(client.sendCommand('LIST_GOALS')):
        error("Failed to send the command 'LIST_GOALS' to the server")

    while True:
        response = client.waitResponse()
        if response is None:
            error("Failed to wait for a response to the command 'LIST_GOALS' from the server")

        if response.name == 'GOAL':
           if len(response.parameters) != 1:
               error("Invalid response to the command 'LIST_GOALS' from the server: GOAL %s" % str(response.parameters))
        elif response.name == 'END_LIST_GOALS':
            break
        else:
            error("Unknown response to the command 'LIST_GOALS' from the server: %s" % response.toString())


    output('TEST: Environments listing')

    if not(client.sendCommand(pymash.Message('LIST_ENVIRONMENTS', ['ReachBlueCell']))):
        error("Failed to send the command 'LIST_ENVIRONMENTS' to the server")

    while True:
        response = client.waitResponse()
        if response is None:
            error("Failed to wait for a response to the command 'LIST_ENVIRONMENTS' from the server")

        if response.name == 'ENVIRONMENT':
           if len(response.parameters) != 1:
               error("Invalid response to the command 'LIST_ENVIRONMENTS' from the server: ENVIRONMENT %s" % str(response.parameters))
        elif response.name == 'END_LIST_ENVIRONMENTS':
            break
        else:
            error("Unknown response to the command 'LIST_ENVIRONMENTS' from the server: %s" % response.toString())


    output('TEST: Task selection')

    if not(client.sendCommand(pymash.Message('INITIALIZE_TASK', ['ReachBlueCell', 'static4x4']))):
        error("Failed to send the command 'INITIALIZE_TASK' to the server")

    response = client.waitResponse()
    if response is None:
        error("Failed to wait for the response 'AVAILABLE_ACTIONS' from the server")

    CHECK_EQUAL('AVAILABLE_ACTIONS', response.name)
    CHECK_EQUAL(4, len(response.parameters))

    response = client.waitResponse()
    if response is None:
        error("Failed to wait for the response 'AVAILABLE_VIEWS' from the server")

    CHECK_EQUAL('AVAILABLE_VIEWS', response.name)

    if response.parameters is None:
        error("The server doesn't report any view")

    views = []
    for view in response.parameters:
        (name, dimensions) = view.split(':')
        (width, height) = dimensions.split('x')
        CHECK_EQUAL('%s:%dx%d' % (name, int(width), int(height)), view)
        views.append( (name, int(width), int(height)) )

    response = client.waitResponse()
    if response is None:
        error("Failed to wait for the response 'MODE' from the server")

    CHECK_EQUAL('MODE', response.name)
    CHECK_EQUAL(1, len(response.parameters))
    CHECK_EQUAL('STANDARD', response.parameters[0])


    check_request(('BEGIN_TASK_SETUP', []), [('OK', [])])
    check_request(('END_TASK_SETUP', []), [('STATE_UPDATED', [])])
 
    
    output('TEST: Views retrieval')

    for view in views:
        data_size = 8 + 3 * view[1] * view[2]

        check_request(('GET_VIEW', [view[0]]),
                            [('VIEW', [view[0], 'image/mif', data_size]),
                            ])

        image = client.waitData(data_size)
        if image is None:
            error("Failed to retrieve the view '%s'" % view[0])
        

    output('TEST: Perform actions')

    check_request(('ACTION', ['GO_NORTH']),
                        [('REWARD', [-1.0]),
                         ('EVENT', ['Hit north wall']),
                         ('STATE_UPDATED', []),
                        ])

    check_request(('ACTION', ['GO_WEST']),
                        [('REWARD', [-1.0]),
                         ('EVENT', ['Hit west wall']),
                         ('STATE_UPDATED', []),
                        ])

    for i in range(0, 3):
        check_request(('ACTION', ['GO_EAST']),
                            [('REWARD', [0.0]),
                             ('STATE_UPDATED', []),
                            ])

    check_request(('ACTION', ['GO_EAST']),
                        [('REWARD', [-1.0]),
                         ('EVENT', ['Hit east wall']),
                         ('STATE_UPDATED', []),
                        ])


    output('TEST: Reset state')

    check_request(('RESET_TASK', []),
                        [('STATE_UPDATED', []),
                        ])

    check_request(('ACTION', ['GO_NORTH']),
                        [('REWARD', [-1.0]),
                         ('EVENT', ['Hit north wall']),
                         ('STATE_UPDATED', []),
                        ])

    check_request(('ACTION', ['GO_WEST']),
                        [('REWARD', [-1.0]),
                         ('EVENT', ['Hit west wall']),
                         ('STATE_UPDATED', []),
                        ])
    
    
    output('TEST: Perform more actions')

    for i in range(0, 3):
        check_request(('ACTION', ['GO_SOUTH']),
                            [('REWARD', [0.0]),
                             ('STATE_UPDATED', []),
                            ])

    check_request(('ACTION', ['GO_SOUTH']),
                        [('REWARD', [-1.0]),
                         ('EVENT', ['Hit south wall']),
                         ('STATE_UPDATED', []),
                        ])

    for i in range(0, 2):
        check_request(('ACTION', ['GO_EAST']),
                            [('REWARD', [0.0]),
                             ('STATE_UPDATED', []),
                            ])

    check_request(('ACTION', ['GO_EAST']),
                        [('REWARD', [10.0]),
                         ('EVENT', ['Goal reached']),
                         ('FINISHED', []),
                        ])

    
    output('TEST: End of session')

    check_request(('DONE', []), [('GOODBYE', [])])
    
    # Close the connection
    client.close()
    if server is not None:
        os.kill(server.pid, signal.SIGTERM)

    output('Done')
