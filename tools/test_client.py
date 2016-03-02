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


# Parameters handling
if (len(sys.argv) != 4) and (len(sys.argv) != 3):
    print "Usage: %s <hostname> <port> [<input_file>]" % sys.argv[0]
    sys.exit(-1)


# Create the client and connect to the server
client = Client()
if not(client.connect(sys.argv[1], int(sys.argv[2]))):
    print 'Failed to connect to the server'
    sys.exit(-1)


if len(sys.argv) == 3:
    stdscr = curses.initscr()
    stdscr.keypad(1)

    stdscr.addstr('Commands: (S)end a command, (R)eceive response, (Q)uit\n')

    try:
    
        while True:
            curses.raw()
            curses.noecho()
            c = stdscr.getkey()
            curses.echo()
            curses.noraw()
    
            if c == 's':
                stdscr.addstr('> ')
                command_line = stdscr.getstr()
                parts = command_line.split(' ')

                if not(client.sendCommand(Message(parts[0], parts[1:]))):
                    stdscr.addstr("! Failed to send command '%s' to the server\n" % parts[0])

            elif c == 'r':
                stdscr.addstr('< ')
                stdscr.refresh()
                response = client.waitResponse()
                if response is not None:
                    stdscr.addstr('%s\n' % response.toString())
                else:
                    stdscr.addstr("\n! Failed to wait for response to command '%s' from the server\n" % parts[0])

            elif c == 'q':
                break
    except:
        pass

    curses.endwin();

else:
    inFile = open(sys.argv[3], 'r')
    content = inFile.read()
    inFile.close()
    
    lines = content.split('\n')
    
    for line in lines:
        if line.startswith('>'):
            print line
            command_line = line[1:].strip()
            parts = command_line.split(' ')
            if not(client.sendCommand(Message(parts[0], parts[1:]))):
                print "! Failed to send command '%s' to the server" % parts[0]
                break
        else:
            response = client.waitResponse()
            if response is not None:
                print response.toString()
            else:
                print "\n! Failed to wait for response from the server"
                break
    
client.close()
