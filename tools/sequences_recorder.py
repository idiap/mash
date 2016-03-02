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

import random

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
parser.add_option("--outputdir", action="store", dest="outputdir", default='.',
                  help="Directory where the images, actions and rewards must be written")
parser.add_option("--toppm", action="store_true", dest="toppm", default=False,
                  help="Convert the MIF images in PPM")
parser.add_option("--task_settings", action="store", dest="settings", default='',
                  help="Path to a file containing the task-specific settings")
parser.add_option("--seed", action="store", dest="seed", default=0, type="int",
                  help="Global seed to send to the Application Server")
parser.add_option("--nb_sequences", action="store", dest="nb_sequences", default=100, type="int",
                  help="Number of sequences")
parser.add_option("--nb_max_frames", action="store", dest="nb_max_frames", default=300, type="int",
                  help="Maximum number of frames per sequence")


# Handling of the arguments
(options, args) = parser.parse_args()
if (len(args) != 2):
    parser.print_help()
    sys.exit(1)

hostname = args[0]
port = int(args[1])


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
    record_file = open(os.path.join(options.outputdir, 'index.txt'), 'w')
    record_file.write('RECORDED_TEACHER\n')

    response = client.waitResponse()
    if response is None:
        bail('FAILED (no response)')

    if (response.name != 'AVAILABLE_ACTIONS') or \
       (response.parameters is None) or \
       (len(response.parameters) == 0):
        bail(response.toString())

    record_file.write('%s\n' % response.toString())

    actions = response.parameters

    response = client.waitResponse()
    if response is None:
        bail('FAILED (no response)')

    if (response.name != 'AVAILABLE_VIEWS') or \
       (response.parameters is None) or \
       (len(response.parameters) == 0):
        bail(response.toString())

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

    if not(suggestedActionsAvailable):
        bail('FAILED (No teacher found)')

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
        if (response.name != 'SUGGESTED_ACTION') or (len(response.parameters) != 1):
            bail(response.toString())

        stdscr.addstr('%s\n' % response.toString(), COLOR_YELLOW)

        teacher_action = response.parameters[0]

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

    record_file.write('TRAJECTORY_START\n')
    
    canChooseAction  = True
    imageCounter     = 0
    taskResult       = None
    framesCounter    = 0
    sequencesCounter = 0
   
    follow_teacher = True
    myaction = "Unknown"

    while sequencesCounter < options.nb_sequences:

        # Retrieve the views
        stdscr.addstr('\n')
        stdscr.addstr('Retrieving the views... ')

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

            image_file_name = '%s_%07d.%s' % (view['name'], imageCounter, extension)

            outFile = open(os.path.join(options.outputdir, image_file_name), 'wb')

            if options.toppm and (mime_type == 'image/mif'):
                outFile.write('P6\n%d %d\n255\n' % (view['width'], view['height']))
                outFile.write(data[8:])
            else:
                outFile.write(data)

            outFile.close()
        
            record_file.write(' %s' % image_file_name)

        record_file.write('\n')
        imageCounter += 1

            
        stdscr.addstr('\n\n')

        if not(canChooseAction) or (framesCounter >= options.nb_max_frames):

            stdscr.addstr('Resetting the task...\n\n')

            # Send a reset command
            if not(client.sendCommand('RESET_TASK')):
                bail('FAILED (command not sent)')

            response = client.waitResponse()
            if response is None:
                bail('FAILED (no response)')

            if (response.name != 'SUGGESTED_ACTION') or (len(response.parameters) != 1):
                bail(response.toString())

            teacher_action = response.parameters[0]

            stdscr.addstr('%s\n' % response.toString(), COLOR_YELLOW)

            response = client.waitResponse()
            if response is None:
                bail('FAILED (no response)')

            if response.name != 'STATE_UPDATED':
                bail(response.toString())

            canChooseAction = True
            framesCounter = 0
            sequencesCounter += 1

            if taskResult is not None:
                record_file.write('%s\n' % taskResult)
                taskResult = None

            if sequencesCounter < options.nb_sequences:
                record_file.write('TRAJECTORY_START\n')

        else:

            record_file.write('ACTION %s\n' % teacher_action)

            # Send the action
            #algorithm to add some non teacher action
            randomNumber = random.random()

            if follow_teacher:
                if randomNumber < 0.05 :
                    follow_teacher = False
            else :
                if randomNumber < 0.1 :
                    follow_teacher = True
            
            if randomNumber < 0.1 :
                myaction = random.randint(0,len(actions)-1)
            if follow_teacher:
                if not(client.sendCommand(Message('ACTION', [teacher_action]))):
                    bail('FAILED (command not sent)')
            else:
                if not(client.sendCommand(Message('ACTION', [actions[myaction]]))):
                    bail('FAILED (command not sent)')


            while True:
                response = client.waitResponse()
                if response is None:
                    bail('FAILED (no response)')

                stdscr.addstr('%s\n' % response.toString(), COLOR_YELLOW)

                if response.name == 'REWARD':
                    record_file.write('REWARD %f\n' % float(response.parameters[0]))
                
                elif response.name == 'SUGGESTED_ACTION':
                    teacher_action = response.parameters[0]
                
                elif (response.name == 'FINISHED') or (response.name == 'FAILED'):
                    taskResult = response.name
                    canChooseAction = False
                    break

                elif response.name == 'STATE_UPDATED':
                    canChooseAction = True
                    break
        
        framesCounter += 1

        sys.stdout.flush()


except KeyboardInterrupt:
    if taskResult is not None:
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
