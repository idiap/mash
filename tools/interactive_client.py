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


from pymash import Client
from pymash import Message
import sys
import os
import curses
import traceback
from optparse import OptionParser

# Global variables
stdscr = None
COLOR_RED = None
COLOR_GREEN = None
COLOR_YELLOW = None


def bail(text, args=[]):
    global stdscr
    
    stdscr.addstr(text, COLOR_RED)
    for arg in args:
        stdscr.addstr(' %s' % arg, COLOR_RED)
    stdscr.addstr('\n')
 
    curses.raw()
    curses.noecho()
    c = stdscr.getkey()
    curses.echo()
    curses.noraw()
 
    raise KeyboardInterrupt


# Setup of the command-line arguments parser
parser = OptionParser("Usage: %prog <hostname> <port>", version="%prog 1.1")
parser.add_option("--record", action="store_true", dest="record", default=False,
                  help="Enable the recording of all the images, actions and rewards")
parser.add_option("--outputdir", action="store", dest="outputdir", default='.',
                  help="Directory where the images, actions and rewards must be written")
parser.add_option("--toppm", action="store_true", dest="toppm", default=False,
                  help="Convert the MIF images in PPM")
parser.add_option("--mapping", action="store", dest="mapping", default='',
                  help="Path to a keyboard mapping file, that associates keys to actions")
parser.add_option("--task_settings", action="store", dest="settings", default='',
                  help="Path to a file containing the task-specific settings")
parser.add_option("--seed", action="store", dest="seed", default=0, type="int",
                  help="Global seed to send to the Application Server")


# Handling of the arguments
(options, args) = parser.parse_args()
if (len(args) != 2):
    parser.print_help()
    sys.exit(1)

hostname = args[0]
port = int(args[1])


mapping = {}
if options.mapping != '':
    inFile = open(options.mapping, 'r')
    lines = inFile.readlines()
    inFile.close()
    
    for line in lines:
        if len(line.strip()) == 0:
            continue
        
        (action, key) = line.split(':')
        action = action.strip()
        key = key.strip()
        mapping[action] = key
        

# Initialize curses
stdscr = curses.initscr()
stdscr.keypad(1)
stdscr.scrollok(1)
stdscr.idlok(1)

curses.start_color()
curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)
curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK)
curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)

COLOR_RED = curses.color_pair(1)
COLOR_GREEN = curses.color_pair(2)
COLOR_YELLOW = curses.color_pair(3)


taskResult = None

try:
    # Create the output directory if necessary
    if not(os.path.exists(options.outputdir)):
        os.makedirs(options.outputdir)

    # Create the client and connect to the server
    stdscr.addstr('Establishing the connection with the server...')
    client = Client()
    if not(client.connect(hostname, port)):
        bail(' FAILED')

    stdscr.addstr('\n')


    # Check that the server is an interactive one, and isn't busy
    stdscr.addstr('Checking the status of the server... ')
    if not(client.sendCommand(Message('STATUS', []))):
        bail('FAILED (command not sent)')

    response = client.waitResponse()
    if response is None:
        bail('FAILED (no response)')

    if response.name != 'READY':
        bail(response.toString())

    stdscr.addstr('\n')

    stdscr.addstr('Checking the type of the server... ')
    if not(client.sendCommand(Message('INFO', []))):
        bail('FAILED (command not sent)')

    response = client.waitResponse()
    if response is None:
        bail('FAILED (no response)')

    if (response.name != 'TYPE') or (len(response.parameters) != 1) or (response.parameters[0] != 'ApplicationServer'):
        bail(response.toString())

    stdscr.addstr('\n')

    stdscr.addstr('Checking the subtype of the server... ')

    response = client.waitResponse()
    if response is None:
        bail('FAILED (no response)')

    if (response.name != 'SUBTYPE') or (len(response.parameters) != 1) or (response.parameters[0] != 'Interactive'):
        bail(response.toString())

    stdscr.addstr('\n')

    stdscr.addstr('Checking the protocol supported by the server... ')

    response = client.waitResponse()
    if response is None:
        bail('FAILED (no response)')

    if (response.name != 'PROTOCOL') or (len(response.parameters) != 1) or (str(response.parameters[0]) not in ['1.4', '1.3', '1.2']):
        bail(response.toString())

    canUseRecordings = (str(response.parameters[0]) == '1.4')

    stdscr.addstr('\n')

    while True:
        # Sends the global seed
        if options.seed != 0:
            stdscr.addstr('Sends the global seed... ')
            if not(client.sendCommand(Message('USE_GLOBAL_SEED', [options.seed]))):
                bail('FAILED (command not sent)')

            response = client.waitResponse()
            if response is None:
                bail('FAILED (no response)')

            if response != 'OK':
                bail(response.toString())

            stdscr.addstr('\n')

        # Retrieve the list of goals
        goals = []

        stdscr.addstr('Retrieving the list of goals... ')
        if not(client.sendCommand('LIST_GOALS')):
            bail('FAILED (command not sent)')

        while (True):
            response = client.waitResponse()
            if response is None:
                bail('FAILED (no response)')

            if response.name == 'END_LIST_GOALS':
                break

            elif response.name == 'GOAL':
                if len(response.parameters) != 1:
                    bail(response.toString())
                goals.append(response.parameters[0])
    
            else:
                bail(response.toString())

        if len(goals) == 0:
            bail('ERROR No goal reported')
        
        stdscr.addstr('\n\n')


        # Let the user select a goal
        stdscr.addstr('--------------------------------------------------\n', COLOR_GREEN)
        stdscr.addstr('Select a goal:\n', COLOR_GREEN)

        counter = 1
        for goal in goals:
            (y, x) = stdscr.getyx()
            stdscr.addstr(y, x + 4, '%d' % counter, COLOR_GREEN)
            stdscr.addstr(y, x + 10, goal, COLOR_GREEN)
            stdscr.addstr('\n')
            counter += 1

        while True:
            stdscr.addstr('\n')
            stdscr.addstr('Your choice: ')
            choice = stdscr.getstr()

            try:
                selected_goal = int(choice) - 1
            except:
                stdscr.addstr('Invalid goal!\n', COLOR_RED)
                continue
    
            if (selected_goal < 0) or (selected_goal >= len(goals)):
                stdscr.addstr('Invalid goal!\n', COLOR_RED)
            else:
                break


        stdscr.addstr('\n\n')

        # Retrieve the list of environments
        environments = []

        stdscr.addstr('Retrieving the list of environments... ')
        if not(client.sendCommand(Message('LIST_ENVIRONMENTS', [goals[selected_goal]]))):
            bail('FAILED (command not sent)')

        while (True):
            response = client.waitResponse()
            if response is None:
                bail('FAILED (no response)')

            if response.name == 'END_LIST_ENVIRONMENTS':
                break

            elif response.name == 'ENVIRONMENT':
                if len(response.parameters) != 1:
                    bail(response.toString())
                environments.append(response.parameters[0])
    
            else:
                bail(response.toString())

        if len(environments) == 0:
            bail('ERROR No environment reported')
        
        stdscr.addstr('\n\n')


        # Let the user select an environment
        stdscr.addstr('--------------------------------------------------\n', COLOR_GREEN)
        stdscr.addstr('Select an environment:\n', COLOR_GREEN)

        counter = 1
        for environment in environments:
            (y, x) = stdscr.getyx()
            stdscr.addstr(y, x + 4, '%d' % counter, COLOR_GREEN)
            stdscr.addstr(y, x + 10, environment, COLOR_GREEN)
            stdscr.addstr('\n')
            counter += 1

        while True:
            stdscr.addstr('\n')
            stdscr.addstr('Your choice: ')
            choice = stdscr.getstr()

            try:
                selected_environment = int(choice) - 1
            except:
                stdscr.addstr('Invalid environment!\n', COLOR_RED)
                continue
    
            if (selected_environment < 0) or (selected_environment >= len(environments)):
                stdscr.addstr('Invalid environment!\n', COLOR_RED)
            else:
                break


        stdscr.addstr('\n\n')

        # Initialize the task
        stdscr.addstr('Initialization of the task...\n\n')
        if not(client.sendCommand(Message('INITIALIZE_TASK', [goals[selected_goal], environments[selected_environment]]))):
            bail('FAILED (command not sent)')

        # Create the record file
        if options.record:
            record_file = open(os.path.join(options.outputdir, 'index.txt'), 'w')
            record_file.write('RECORDED_TEACHER\n')

        response = client.waitResponse()
        if response is None:
            bail('FAILED (no response)')

        if (response.name != 'AVAILABLE_ACTIONS') or \
           (response.parameters is None) or \
           (len(response.parameters) == 0):
            bail(response.toString())

        if options.record:
            record_file.write('%s\n' % response.toString())

        actions = response.parameters

        response = client.waitResponse()
        if response is None:
            bail('FAILED (no response)')

        if (response.name != 'AVAILABLE_VIEWS') or \
           (response.parameters is None) or \
           (len(response.parameters) == 0):
            bail(response.toString())

        if options.record:
            record_file.write('%s\n' % response.toString())

        views = []
        for view in response.parameters:
            (name, dimensions) = view.split(':')
            (width, height) = dimensions.split('x')

            details = {
                'name': name,
                'width': int(width),
                'height': int(height),
            }

            views.append(details)

        stdscr.addstr('%d actions, %d views\n\n' % (len(actions), len(views)), COLOR_YELLOW)

        suggestedActionsAvailable = False
        if canUseRecordings:
            response = client.waitResponse()
            if response is None:
                bail('FAILED (no response)')

            if (response.name != 'MODE') or (len(response.parameters) != 1):
                bail(response.toString())

            stdscr.addstr('%s\n' % response.toString(), COLOR_YELLOW)
            
            suggestedActionsAvailable = (response.parameters[0] != 'STANDARD')

        if not(client.sendCommand('BEGIN_TASK_SETUP')):
            bail('FAILED (command not sent)')

        response = client.waitResponse()
        if response is None:
            bail('FAILED (no response)')

        if (response.name != 'OK'):
            bail(response.toString())

        if options.settings != '':
            inFile = open(options.settings, 'r')
            lines = inFile.readlines()
            inFile.close()

            for line in lines:
                line = line.strip()
                if len(line) == 0:
                    continue
            
                parts = line.split(' ')

                if not(client.sendCommand(Message(parts[0], parts[1:]))):
                    bail('FAILED (task-specific setting not sent)')

                response = client.waitResponse()
                if response is None:
                    bail('FAILED (no response)')

                if (response.name != 'OK'):
                    bail(response.toString())

        if not(client.sendCommand('END_TASK_SETUP')):
            bail('FAILED (command not sent)')

        response = client.waitResponse()
        if response is None:
            bail('FAILED (no response)')

        if canUseRecordings:
            if suggestedActionsAvailable:
                if (response.name != 'SUGGESTED_ACTION') or (len(response.parameters) != 1):
                    bail(response.toString())

                stdscr.addstr('%s\n' % response.toString(), COLOR_YELLOW)

                response = client.waitResponse()
                if response is None:
                    bail('FAILED (no response)')

                if (response.name != 'STATE_UPDATED'):
                    bail(response.toString())
        else:
            if (response.name != 'OK'):
                bail(response.toString())

        stdscr.addstr('\n')
        
        # Setup the mapping of the actions to keys
        counter = 2
        keys_column_width = 4

        mapping_copy = mapping
        mapping = {}
        for (action, entry) in mapping_copy.items():
            if action in actions:
                mapping[action] = entry

        for action in actions:
            if not(mapping.has_key(action)):
                mapping[action] = str(counter)
                counter += 1
        
        
        if options.record:
            record_file.write('TRAJECTORY_START\n')
        
        didReset = False
        canChooseAction = True
        imageCounter = 0
        taskResult = None
        while not(didReset):

            didReset = False

            # Retrieve the views
            stdscr.addstr('\n')
            stdscr.addstr('Retrieving the views... ')

            if options.record:
                record_file.write('IMAGES')

            for view in views:
                if not(client.sendCommand(Message('GET_VIEW', [view['name']]))):
                    bail('FAILED (command not sent)')

                response = client.waitResponse()
                if response is None:
                    bail('FAILED (no response)')

                if (response.name != 'VIEW') or (len(response.parameters) != 3) or (response.parameters[0] != view['name']):
                    bail(response.toString())
            
                mime_type = response.parameters[1]
                size = int(response.parameters[2])

                data = client.waitData(size)
                if data is None:
                    bail('FAILED (no data)')

                extension = mime_type[6:]

                if options.toppm and (mime_type == 'image/mif'):
                    extension = 'ppm'

                if options.record:
                    image_file_name = '%s_%04d.%s' % (view['name'], imageCounter, extension)
                else:
                    image_file_name = '%s.%s' % (view['name'], extension)

                outFile = open(os.path.join(options.outputdir, image_file_name), 'wb')

                if options.toppm and (mime_type == 'image/mif'):
                    outFile.write('P6\n%d %d\n255\n' % (view['width'], view['height']))
                    outFile.write(data[8:])
                else:
                    outFile.write(data)

                outFile.close()
            
                if options.record:
                    record_file.write(' %s' % image_file_name)

            if options.record:
                record_file.write('\n')
            imageCounter += 1

                
            stdscr.addstr('\n\n')

            # Let the user select an action
            stdscr.addstr('--------------------------------------------------\n', COLOR_GREEN)
            stdscr.addstr('Select an action:\n', COLOR_GREEN)

            (y, x) = stdscr.getyx()
            stdscr.addstr(y, x + 4, '0', COLOR_GREEN)
            stdscr.addstr(y, x + 4 + keys_column_width, 'Reset the server', COLOR_GREEN)
            stdscr.addstr('\n')

            (y, x) = stdscr.getyx()
            stdscr.addstr(y, x + 4, '1', COLOR_GREEN)
            stdscr.addstr(y, x + 4 + keys_column_width, 'Reset the task', COLOR_GREEN)
            stdscr.addstr('\n')

            if canChooseAction:
                for (action, entry) in mapping.items():
                    (y, x) = stdscr.getyx()
                    stdscr.addstr(y, x + 4, entry, COLOR_GREEN)
                    stdscr.addstr(y, x + 4 + keys_column_width, action, COLOR_GREEN)
                    stdscr.addstr('\n')

            while True:
                curses.cbreak()
                curses.noecho()

                try:
                    c = stdscr.getkey()
                except KeyboardInterrupt:
                    curses.echo()
                    curses.nocbreak()
                    raise

                curses.echo()
                curses.nocbreak()

                selected_action = None

                if c == '0':
                    selected_action = 'RESET'
                elif c == '1':
                    selected_action = 'RESET_TASK'
                elif canChooseAction:
                    for (action, entry) in mapping.items():
                        if c == entry:
                            selected_action = action
                            break

                if selected_action is not None:
                    break

                curses.flash()

            stdscr.addstr('\n')

            if selected_action == 'RESET':

                stdscr.addstr('Resetting the server...')

                # Send a reset command
                if not(client.sendCommand('RESET')):
                    bail('FAILED (command not sent)')

                response = client.waitResponse()
                if response is None:
                    bail('FAILED (no response)')

                if response.name != 'OK':
                    bail(response.toString())
                
                didReset = True
                canChooseAction = True

                stdscr.addstr('\n')

            elif selected_action == 'RESET_TASK':

                stdscr.addstr('Resetting the task...\n\n')

                # Send a reset command
                if not(client.sendCommand('RESET_TASK')):
                    bail('FAILED (command not sent)')

                if suggestedActionsAvailable:
                    response = client.waitResponse()
                    if response is None:
                        bail('FAILED (no response)')

                    if (response.name != 'SUGGESTED_ACTION') or (len(response.parameters) != 1):
                        bail(response.toString())

                    stdscr.addstr('%s\n' % response.toString(), COLOR_YELLOW)

                response = client.waitResponse()
                if response is None:
                    bail('FAILED (no response)')

                if response.name != 'STATE_UPDATED':
                    bail(response.toString())

                canChooseAction = True

                if options.record and (taskResult is not None):
                    record_file.write('%s\n' % taskResult)
                    taskResult = None

                if options.record:
                    record_file.write('TRAJECTORY_START\n')

            elif canChooseAction:

                if options.record:
                    record_file.write('ACTION %s\n' % selected_action)

                # Send the action
                if not(client.sendCommand(Message('ACTION', [selected_action]))):
                    bail('FAILED (command not sent)')

                while True:
                    response = client.waitResponse()
                    if response is None:
                        bail('FAILED (no response)')

                    stdscr.addstr('%s\n' % response.toString(), COLOR_YELLOW)

                    if response.name == 'REWARD':
                        if options.record:
                            record_file.write('REWARD %f\n' % float(response.parameters[0]))
                    
                    elif (response.name == 'FINISHED') or (response.name == 'FAILED'):
                        taskResult = response.name
                        canChooseAction = False
                        break

                    elif response.name == 'STATE_UPDATED':
                        canChooseAction = True
                        break


except KeyboardInterrupt:
    if options.record and (taskResult is not None):
        record_file.write('%s\n' % taskResult)

except:
    print traceback.format_exc()
    stdscr.addstr('\n' + traceback.format_exc(), COLOR_RED)

    curses.raw()
    curses.noecho()
    c = stdscr.getkey()
    curses.echo()
    curses.noraw()


curses.endwin()
    
client.close()
