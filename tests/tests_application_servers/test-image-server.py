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
# This script is used to test the implementation of the Image Server
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
            CHECK_EQUAL(str(expected_args[i]), str(response.parameters[i]))


##################################### MAIN #####################################

if __name__ == "__main__":

    # Setup of the command-line arguments parser
    usage = """Usage: %prog --server=PATH [options]
       %prog [options] ADDRESS PORT

If the --server option is used, an application server is started on the local
machine. Otherwise the program will attempt to connect to the specified address."""
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("-q", "--quiet", action="store_true", default=False,
                      dest="quiet", help="Don't write non-error messages to the console output")
    parser.add_option("--pymash", action="store", default="pymash", type="string",
                      dest="pymash_path", help="Path to the pymash module")
    parser.add_option("--server", action="store", default=None, type="string", metavar="PATH",
                      dest="server_path", help="Path to the application server to execute")
    parser.add_option("--serverconfig", action="store", default="image-server-config", type="string",
                      dest="server_config", help="Path to the server configuration file")

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
    server_address = args[0]
    server_port = int(args[1])

    if CONFIGURATION.server_path is not None:
        command = "%s --config=%s --host=%s --port=%d" % (os.path.abspath(CONFIGURATION.server_path),
                                                          os.path.abspath(CONFIGURATION.server_config),
                                                          server_address, server_port)
        server = subprocess.Popen(command.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        time.sleep(1)

        if server.poll():
            output = server.stdout.read()
            server = None
            error('Failed to start the application server, output: \n' + output)
        
    # Create a client and connect to the application server
    client = pymash.Client()
    if not(client.connect(server_address, server_port)):
        error('Failed to connect to the application server')
    
    # Test that the protocol is respected
    output('TEST: Info retrieval')

    check_request(('STATUS', []), [('READY', [])])
    check_request(('INFO', []), [('TYPE', ['ApplicationServer']),
                                 ('SUBTYPE', ['Images']),
                                 ('PROTOCOL', ['1.2'])
                                ])

                                 
    output('TEST: Database selection')

    check_request(('SELECT_DATABASE', ['coil-100']),
                        [('NB_IMAGES', [7200]),
                         ('NB_LABELS', [100]),
                         ('NB_OBJECTS', [7200])
                        ])

    
    output('TEST: Preferred image size retrieval')

    check_request(('REPORT_PREFERRED_IMAGE_SIZE', [74]),
                        [('PREFERRED_IMAGE_SIZE', [127, 127]),
                        ])

    output('TEST: Preferred ROI size retrieval')

    check_request(('REPORT_PREFERRED_ROI_SIZE', [74]),
                        [('PREFERRED_ROI_SIZE', [127]),
                        ])

    output('TEST: URLs prefix retrieval')

    check_request(('REPORT_URL_PREFIX', []),
                        [('URL_PREFIX', ['http://test.org/db/coil-100/']),
                        ])

    output('TEST: Image retrieval')

    check_request(('IMAGE', [74]),
                        [('IMAGE_NAME', ['obj2__10.png']),
                        ])


    output('TEST: Invalid image retrieval')

    check_request(('IMAGE', [10000]),
                        [('UNKNOWN_IMAGE', []),
                        ])

    check_request(('IMAGE', [-1]),
                        [('UNKNOWN_IMAGE', []),
                        ])


    output('TEST: Label names retrieval')

    response = []
    for i in range(0, 100):
        response.append(('LABEL_NAME', [str(i)]))
    response.append(('END_LIST_LABEL_NAMES', []))

    check_request(('LIST_LABEL_NAMES', [i]), response)


    output('TEST: Object list retrieval')

    if not(client.sendCommand('LIST_OBJECTS')):
        error("Failed to send the command 'LIST_OBJECTS' to the server")

    for i in range(0, 7200):
        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'IMAGE' from the server")

        CHECK_EQUAL('IMAGE', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL(i, int(response.parameters[0]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'IMAGE_SIZE' from the server")

        CHECK_EQUAL('IMAGE_SIZE', response.name)
        CHECK_EQUAL(2, len(response.parameters))
        CHECK_EQUAL(127, int(response.parameters[0]))
        CHECK_EQUAL(127, int(response.parameters[1]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'SET' from the server")

        CHECK_EQUAL('SET', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL('NONE', response.parameters[0])

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'NB_OBJECTS' from the server")

        CHECK_EQUAL('NB_OBJECTS', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL(1, int(response.parameters[0]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'OBJECT_LABEL' from the server")

        CHECK_EQUAL('OBJECT_LABEL', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL(i / 72, int(response.parameters[0]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'OBJECT_COORDINATES' from the server")

        CHECK_EQUAL('OBJECT_COORDINATES', response.name)
        CHECK_EQUAL(4, len(response.parameters))
        CHECK_EQUAL(0, int(response.parameters[0]))
        CHECK_EQUAL(0, int(response.parameters[1]))
        CHECK_EQUAL(126, int(response.parameters[2]))
        CHECK_EQUAL(126, int(response.parameters[3]))

    response = client.waitResponse()
    if response is None:
        error("Failed to wait for response 'END_LIST_OBJECTS' from the server")

    CHECK_EQUAL('END_LIST_OBJECTS', response.name)
    CHECK_EQUAL(None, response.parameters)


    output('TEST: Labels enabling (simple enumeration, with background images)')

    check_request(('ENABLE_LABELS', [10, 20, 30]),
                        [('NB_IMAGES', [7200]),
                         ('NB_LABELS', [3]),
                         ('NB_OBJECTS', [72 * 3])
                        ])


    output('TEST: Labels enabling (using ranges, with background images)')

    check_request(('ENABLE_LABELS', ['10-15', '20-22', 30]),
                        [('NB_IMAGES', [7200]),
                         ('NB_LABELS', [10]),
                         ('NB_OBJECTS', [72 * 10])
                        ])


    output('TEST: Background images disabling')

    check_request(('DISABLE_BACKGROUND_IMAGES', []),
                        [('NB_IMAGES', [72 * 10]),
                         ('NB_LABELS', [10]),
                         ('NB_OBJECTS', [72 * 10])
                        ])


    output('TEST: Background images enabling')

    check_request(('ENABLE_BACKGROUND_IMAGES', []),
                        [('NB_IMAGES', [7200]),
                         ('NB_LABELS', [10]),
                         ('NB_OBJECTS', [72 * 10])
                        ])
    

    output('TEST: Enabled labels reset')

    check_request(('RESET_ENABLED_LABELS', []),
                        [('NB_IMAGES', [7200]),
                         ('NB_LABELS', [100]),
                         ('NB_OBJECTS', [7200])
                        ])
    

    output('TEST: Object list retrieval (with some labels enabled, no background image)')

    check_request(('DISABLE_BACKGROUND_IMAGES', []),
                        [('NB_IMAGES', [7200]),
                         ('NB_LABELS', [100]),
                         ('NB_OBJECTS', [7200])
                        ])

    check_request(('ENABLE_LABELS', [10, 20, 30]),
                        [('NB_IMAGES', [72 * 3]),
                         ('NB_LABELS', [3]),
                         ('NB_OBJECTS', [72 * 3])
                        ])
    
    if not(client.sendCommand('LIST_OBJECTS')):
        error("Failed to send the command 'LIST_OBJECTS' to the server")

    for i in range(0, 72 * 3):
        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'IMAGE' from the server")

        CHECK_EQUAL('IMAGE', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL(i, int(response.parameters[0]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'IMAGE_SIZE' from the server")

        CHECK_EQUAL('IMAGE_SIZE', response.name)
        CHECK_EQUAL(2, len(response.parameters))
        CHECK_EQUAL(127, int(response.parameters[0]))
        CHECK_EQUAL(127, int(response.parameters[1]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'SET' from the server")

        CHECK_EQUAL('SET', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL('NONE', response.parameters[0])

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'NB_OBJECTS' from the server")

        CHECK_EQUAL('NB_OBJECTS', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL(1, int(response.parameters[0]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'OBJECT_LABEL' from the server")

        CHECK_EQUAL('OBJECT_LABEL', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL(i / 72, int(response.parameters[0]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'OBJECT_COORDINATES' from the server")

        CHECK_EQUAL('OBJECT_COORDINATES', response.name)
        CHECK_EQUAL(4, len(response.parameters))
        CHECK_EQUAL(0, int(response.parameters[0]))
        CHECK_EQUAL(0, int(response.parameters[1]))
        CHECK_EQUAL(126, int(response.parameters[2]))
        CHECK_EQUAL(126, int(response.parameters[3]))

    response = client.waitResponse()
    if response is None:
        error("Failed to wait for response 'END_LIST_OBJECTS' from the server")

    CHECK_EQUAL('END_LIST_OBJECTS', response.name)
    CHECK_EQUAL(None, response.parameters)
    

    output('TEST: Object list retrieval (with some labels enabled, with background images)')

    check_request(('ENABLE_BACKGROUND_IMAGES', []),
                        [('NB_IMAGES', [7200]),
                         ('NB_LABELS', [3]),
                         ('NB_OBJECTS', [72 * 3])
                        ])
    
    if not(client.sendCommand('LIST_OBJECTS')):
        error("Failed to send the command 'LIST_OBJECTS' to the server")

    for i in range(0, 7200):
        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'IMAGE' from the server")

        CHECK_EQUAL('IMAGE', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL(i, int(response.parameters[0]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'IMAGE_SIZE' from the server")

        CHECK_EQUAL('IMAGE_SIZE', response.name)
        CHECK_EQUAL(2, len(response.parameters))
        CHECK_EQUAL(127, int(response.parameters[0]))
        CHECK_EQUAL(127, int(response.parameters[1]))

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'SET' from the server")

        CHECK_EQUAL('SET', response.name)
        CHECK_EQUAL(1, len(response.parameters))
        CHECK_EQUAL('NONE', response.parameters[0])

        response = client.waitResponse()
        if response is None:
            error("Failed to wait for response 'NB_OBJECTS' from the server")

        if ((i >= 10 * 72) and (i < 11 * 72)) or \
           ((i >= 20 * 72) and (i < 21 * 72)) or \
           ((i >= 30 * 72) and (i < 31 * 72)):
            CHECK_EQUAL('NB_OBJECTS', response.name)
            CHECK_EQUAL(1, len(response.parameters))
            CHECK_EQUAL(1, int(response.parameters[0]))

            response = client.waitResponse()
            if response is None:
                error("Failed to wait for response 'OBJECT_LABEL' from the server")

            CHECK_EQUAL('OBJECT_LABEL', response.name)
            CHECK_EQUAL(1, len(response.parameters))

            if i < 11 * 72:
                CHECK_EQUAL(0, int(response.parameters[0]))
            elif i < 21 * 72:
                CHECK_EQUAL(1, int(response.parameters[0]))
            else:
                CHECK_EQUAL(2, int(response.parameters[0]))

            response = client.waitResponse()
            if response is None:
                error("Failed to wait for response 'OBJECT_COORDINATES' from the server")

            CHECK_EQUAL('OBJECT_COORDINATES', response.name)
            CHECK_EQUAL(4, len(response.parameters))
            CHECK_EQUAL(0, int(response.parameters[0]))
            CHECK_EQUAL(0, int(response.parameters[1]))
            CHECK_EQUAL(126, int(response.parameters[2]))
            CHECK_EQUAL(126, int(response.parameters[3]))
        else:
            CHECK_EQUAL('NB_OBJECTS', response.name)
            CHECK_EQUAL(1, len(response.parameters))
            CHECK_EQUAL(0, int(response.parameters[0]))
            
    response = client.waitResponse()
    if response is None:
        error("Failed to wait for response 'END_LIST_OBJECTS' from the server")

    CHECK_EQUAL('END_LIST_OBJECTS', response.name)
    CHECK_EQUAL(None, response.parameters)


    output('TEST: Image retrieval (with some labels enabled, with background images)')

    check_request(('IMAGE', [74]),
                        [('IMAGE_NAME', ['obj2__10.png']),
                        ])


    output('TEST: Invalid image retrieval (with some labels enabled, with background images)')

    check_request(('IMAGE', [10000]),
                        [('UNKNOWN_IMAGE', []),
                        ])

    check_request(('IMAGE', [-1]),
                        [('UNKNOWN_IMAGE', []),
                        ])


    output('TEST: Image retrieval (with some labels enabled, no background images)')

    check_request(('DISABLE_BACKGROUND_IMAGES', []),
                        [('NB_IMAGES', [72 * 3]),
                         ('NB_LABELS', [3]),
                         ('NB_OBJECTS', [72 * 3])
                        ])

    check_request(('IMAGE', [74]),
                        [('IMAGE_NAME', ['obj21__10.png']),
                        ])


    output('TEST: Invalid image retrieval (with some labels enabled, no background images)')

    check_request(('IMAGE', [72 * 3]),
                        [('UNKNOWN_IMAGE', []),
                        ])

    check_request(('IMAGE', [-1]),
                        [('UNKNOWN_IMAGE', []),
                        ])
    
    
    output('TEST: End of session')

    check_request(('DONE', []), [('GOODBYE', [])])
    
    # Close the connection
    client.close()
    if server is not None:
        os.kill(server.pid, signal.SIGTERM)

    output('Done')
