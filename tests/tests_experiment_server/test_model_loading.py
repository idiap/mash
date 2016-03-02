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
import tarfile
import traceback
from optparse import OptionParser


#################################### GLOBALS ###################################

CONFIGURATION     = None
experiment_server = None
appserver         = None
client            = None


################################## FUNCTIONS ###################################

def write(text):
    if not(CONFIGURATION.quiet):
        print text


def error(text):
    print 'ERROR: %s' % text
    
    if client is not None:
        client.sendCommand('DONE')
        client.close()
    
    if experiment_server is not None:
        os.kill(experiment_server.pid, signal.SIGTERM)
        (output, errors) = experiment_server.communicate()
        print output
        
    if appserver is not None:
        os.kill(appserver.pid, signal.SIGTERM)
        (output, errors) = appserver.communicate()
    
    sys.exit(1)


def sendCommand(client, command, expected_responses):
    from pymash import Message

    write("> %s" % command.toString())

    if not(client.sendCommand(command)):
        error("Failed to send the command '%s' to the server" % command.toString())

    for expected in expected_responses:
        response = Message('NOTIFICATION')

        while (response.name == 'NOTIFICATION'):
            response = client.waitResponse()
            if response is None:
                error("Failed to wait for response to the command '%s' from the server" % command.name)
            write("< %s" % response.toString())

        if response.name != expected:
            if response.name == 'ERROR':
                error("Error received from the server: '%s'" % response.parameters[0])
            else:
                error("Unexpected response from the server: got '%s', expected '%s'" % (response.name, expected))

    return response


def sendCommandAndFile(client, command, filename, expected_responses):
    from pymash import Message

    inFile = open(filename, 'rb')
    inFile.seek(0, os.SEEK_END)
    size = inFile.tell()
    inFile.seek(0, os.SEEK_SET)

    message = Message(command, [size])

    write("> %s" % message.toString())

    if not(client.sendCommand(message)):
        error("Failed to send the command '%s' to the server" % message.toString())

    max_size = 10 * 1024
    while size > 0:
        nb = min(size, max_size)
        data = inFile.read(size)
        client.sendData(data)
        size -= nb

    for expected in expected_responses:
        response = Message('NOTIFICATION')

        while (response.name == 'NOTIFICATION'):
            response = client.waitResponse()
            if response is None:
                error("Failed to wait for response to the command '%s' from the server" % message.name)
            write("< %s" % response.toString())

        if response.name != expected:
            if response.name == 'ERROR':
                error("Error received from the server: '%s'" % response.parameters[0])
            else:
                error("Unexpected response from the server: got '%s', expected '%s'" % (response.name, expected))

    inFile.close()

    return response


##################################### MAIN #####################################

def process(args):
    global experiment_server
    global appserver
    global client

    script_dir = os.path.abspath(os.path.dirname(sys.argv[0]))

    appserver_type          = args[0]
    appserver_dir           = args[1]
    experiment_server_dir   = args[2]
    settings_file           = args[3]
    use_sandboxing          = (args[4] == 'on')
    use_command_line        = (args[5] == 'on')
    model_file              = args[6]
    
    if len(args) == 8:
        internal_data_file  = args[7]
    else:
        internal_data_file  = None

    # Load the pymash module
    sys.path.append(os.path.join(script_dir, '../../'))
    from pymash import Client
    from pymash import NetworkUtils
    from pymash import Message

    # Start the Application Server
    write('Starting the Application Server...')
    
    if appserver_type == 'image':
        port = 11010
    else:
        port = 11110

    client = Client()
    if client.connect('127.0.0.1', port):
        client = None
        error('The Application Server is already running at 127.0.0.1:%d' % port)

    if appserver_type == 'image':
        command = "./image-server.py --config=%s/image-server-config.py --host=127.0.0.1 --port=11010" % script_dir
    elif appserver_type == 'maze':
        command = "./maze-server --host=127.0.0.1 --port=11110"
    else:
        error('Unknown application server type: ' + appserver_type)

    cwd = os.getcwd()
    
    if len(appserver_dir) > 0:
        os.chdir(appserver_dir)
    
    appserver = subprocess.Popen(command.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    while True:
        time.sleep(1)

        if appserver.poll() is not None:
            output = appserver.stdout.read()
            appserver = None
            error('Failed to start the application server, output: \n' + output)
        
        client = Client()
        if client.connect('127.0.0.1', port):
            client.close()
            client = None
            break

        client = None
    
    os.chdir(cwd)

    # Start the Experiment Server
    client = Client()
    if client.connect('127.0.0.1', 10010):
        client = None
        error('The Experiment Server is already running at 127.0.0.1:10010')

    if len(experiment_server_dir) > 0:
        os.chdir(experiment_server_dir)

    write('Starting the Experiment Server...')

    command = "./experiment-server --no-compilation --host=127.0.0.1 --port=10010"

    if use_command_line:
        command += ' --predictor-model=%s/%s' % (script_dir, model_file)
        
        if internal_data_file is not None:
            command += ' --predictor-data=%s/%s' % (script_dir, internal_data_file)
    
    if not(use_sandboxing):
        command += " --no-sandboxing"

    experiment_server = subprocess.Popen(command.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    # Establish a connection with the Experiment Server
    write('Establishing a connection with the Experiment Server...')

    while True:
        time.sleep(1)

        if experiment_server.poll() is not None:
            output = experiment_server.stdout.read()
            experiment_server = None
            error('Failed to start the Experiment Server, output: \n' + output)
        
        client = Client()
        if client.connect('127.0.0.1', 10010):
            break

        client = None

    # Load the settings file for the Experiment Server
    inFile = open(os.path.join(script_dir, settings_file), 'r')
    content = inFile.read()
    inFile.close()

    commands = filter(lambda x: (len(x) > 0) and not(x.startswith('#')), map(lambda x: x.strip(), content.split('\n')))

    # Send the commands to the Experiment Server
    for command in commands:
        message = Message.fromString(command)
        
        if not(use_command_line) and (message.name == 'USE_PREDICTOR'):
            sendCommandAndFile(client, 'USE_PREDICTOR_MODEL', '%s/%s' % (script_dir, model_file), ['OK'])
            
            if internal_data_file is not None:
                sendCommandAndFile(client, 'USE_PREDICTOR_INTERNAL_DATA', '%s/%s' % (script_dir, internal_data_file), ['OK'])
        
        sendCommand(client, message, ['OK'])

    if appserver_type == 'image':
        sendCommand(client, Message('TRAIN_PREDICTOR'), ['TRAIN_ERROR'])
        sendCommand(client, Message('TEST_PREDICTOR'), ['TEST_ERROR'])
    else:
        sendCommand(client, Message('TRAIN_PREDICTOR'), ['TRAIN_RESULT'])
        sendCommand(client, Message('TEST_PREDICTOR'), ['TEST_RESULT', 'TEST_SCORE'])

    response = sendCommand(client, Message('REPORT_DATA'), ['DATA'])
    data_size = int(response.parameters[0])

    # Retrieve the data report
    content = client.waitData(data_size)
    if content is None:
        error('Failed to retrieve the error report')

    outFile = open('test_data_report.tar.gz', 'wb')
    outFile.write(content)
    outFile.close()

    write('Cleanup')

    client.sendCommand('DONE')
    client.close()
    client = None

    # Stop the Experiment Server
    if experiment_server is not None:
        os.kill(experiment_server.pid, signal.SIGTERM)
        (output, errors) = experiment_server.communicate()
        experiment_server = None

    # Stop the Application Server
    if appserver is not None:
        os.kill(appserver.pid, signal.SIGTERM)
        (output, errors) = appserver.communicate()
        appserver = None

    os.remove('test_data_report.tar.gz')

    write('Done')


if __name__ == "__main__":
    # Setup of the command-line arguments parser
    usage = "Usage: %prog {image | maze} APPSERVER_DIR EXPERIMENT_SERVER_DIR SETTINGS USE_SANDBOXING USE_COMMAND_LINE MODEL_FILE [INTERNAL_DATA_FILE]"
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("-q", "--quiet", action="store_true", default=False,
                      dest="quiet", help="Don't write non-error messages to the console output")

    # Handling of the arguments
    (CONFIGURATION, args) = parser.parse_args()
    if (len(args) != 7) and (len(args) != 8):
        parser.print_help()
        sys.exit(1)

    try:
        process(args)
    except Exception, e:
        if not(isinstance(e, SystemExit)):
            error('An exception occured:\n' + traceback.format_exc())
