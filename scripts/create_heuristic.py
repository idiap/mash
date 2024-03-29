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
HEURISTICS_FOLDER = 'heuristics'


# Setup of the command-line arguments parser
usage = """Usage: %prog [options] <username> <heuristic name>

Examples:
  %prog JohnDoe EdgeDetector
  %prog --empty JohnDoe EdgeDetector
 
  This will create the file 'heuristics/johndoe/edgedetector.cpp' containing the
  skeleton of a heuristic class named 'EdgeDetector' for you.

  ----------

  %prog --path=../heuristics JohnDoe EdgeDetector

  This will create the file '../heuristics/johndoe/edgedetector.cpp'."""

parser = OptionParser(usage, version="%prog 1.0")
parser.add_option("--empty", action="store_true", dest="empty", default=False,
                  help="Create an empty heuristic: no example code from the 'Identity' heuristic")
parser.add_option("--path", action="store", dest="path", default=HEURISTICS_FOLDER, type='string',
                  help="The heuristic is created in the given directory (default: '%s')" % HEURISTICS_FOLDER)


# Handling of the arguments
(options, args) = parser.parse_args()
if len(args) != 2:
    parser.print_help()
    sys.exit(1)

username = args[0]
heuristic_name = args[1]


# Create the user folder
fullpath = os.path.join(options.path, username.lower())
if not(os.path.exists(fullpath)):
    os.makedirs(fullpath)


# Open the template file
file = open(os.path.join(HEURISTICS_FOLDER, 'template.cpp'), 'r')
content = file.read()
file.close()


# Customize its content
content = content.replace('YOUR_USERNAME', username)
content = content.replace('MyHeuristic', heuristic_name)

start = content.find('// Declaration of the heuristic class')
start = content.find('//', start + 2)
end = content.find('//------', start)
content = content[0:start] + content[end:]

start = content.find('// Creation function of the heuristic')
start = content.find('//', start + 2)
end = content.find('//------', start)
content = content[0:start] + content[end:]

if options.empty:
    start = content.find('::dim()')
    start = content.find('{', start)
    end = content.find('}', start)
    content = content[0:start+1] + '\n    // TODO: Implement it\n    return 0;\n' + content[end:]

    start = content.find('::computeFeature(')
    start = content.find('{', start)
    end = content.find('}', start)
    content = content[0:start+1] + '\n    // TODO: Implement it\n    return 0.0f;\n' + content[end:]


# Save it
fullpath = os.path.join(fullpath, heuristic_name.lower() + '.cpp')
file = open(fullpath, 'w')
file.write(content)
file.close()


# Say something to the user
print "File '%s' successfully created!" % fullpath
