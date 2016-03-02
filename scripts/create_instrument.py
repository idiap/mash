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


import sys
import os
from optparse import OptionParser


# Constants
INSTRUMENTS_FOLDER = 'instruments'


# Setup of the command-line arguments parser
usage = """Usage: %prog [options] <username> <instrument name>

Examples:
  %prog JohnDoe MyInstrument
  %prog --path=../instruments JohnDoe MyInstrument"""


parser = OptionParser(usage, version="%prog 1.0")
parser.add_option("--path", action="store", dest="path", default=INSTRUMENTS_FOLDER, type='string',
                  help="The instrument is created in the given directory (default: '%s')" % INSTRUMENTS_FOLDER)


# Handling of the arguments
(options, args) = parser.parse_args()
if len(args) != 2:
    parser.print_help()
    sys.exit(1)

username = args[0]
instrument_name = args[1]
instrument_path = os.path.join(options.path, username.lower(), instrument_name.lower())


# Create the instrument folder if necessary
if not(os.path.exists(instrument_path)):
    os.makedirs(instrument_path)

# Create the standard subfolders
views_path = os.path.join(instrument_path, 'views')
if not(os.path.exists(views_path)):
    os.makedirs(views_path)

instrument_path = os.path.join(instrument_path, 'instrument')
if not(os.path.exists(instrument_path)):
    os.makedirs(instrument_path)


# Open the template file
file = open(os.path.join(INSTRUMENTS_FOLDER, 'template.cpp'), 'r')
content = file.read()
file.close()


# Customize its content
if username is not None:
    content = content.replace('YOUR_USERNAME', username)
content = content.replace('MyInstrument', instrument_name)

start = content.find('// Declaration of the instrument class')
start = content.find('//', start + 2)
end = content.find('//------', start)
content = content[0:start] + content[end:]

start = content.find('// Creation function of the instrument')
start = content.find('//', start + 2)
end = content.find('//------', start)
content = content[0:start] + content[end:]

diff = len(instrument_name) - len('MyInstrument')

if diff != 0:
    offset = content.find(instrument_name + '::')

    while offset >= 0:
        next_line = content.find('\n', offset) + 1
        while content[next_line] != '{':
            if diff < 0:
                content = content[0:next_line] + content[next_line - diff:]
            else:
                content = content[0:next_line] + ' ' * diff + content[next_line:]

            next_line = content.find('\n', next_line) + 1
        
        offset = content.find(instrument_name + '::', offset + 1)


# Save it
fullpath = os.path.join(instrument_path, instrument_name.lower() + '.cpp')
file = open(fullpath, 'w')
file.write(content)
file.close()


# Say something to the user
print "File '%s' successfully created!" % fullpath
