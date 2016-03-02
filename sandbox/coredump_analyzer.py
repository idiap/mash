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
import subprocess
import re


def cmp_stacktrace_entries(x, y):
    x = int(x.split(' ')[0][1:])
    y = int(y.split(' ')[0][1:])
    return cmp(x, y)



def runDebugger(coredump_file, commands, delimiter, frame_header=False):

    def _processFrameHeader(line):
        parts = filter(lambda x: len(x) > 0, map(lambda x: x.strip(), line.split('\n')))
        return '#' + ' '.join(parts)
        

    p = subprocess.Popen(['gdb', '-c', coredump_file, '-se', 'sandbox', '--batch', '-x', commands],
                          stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, universal_newlines=True)

    (output, errors) = p.communicate()
    if p.returncode == 0:
        try:
            content = output.split('%s_END\n' % delimiter)[0].split('%s_START\n' % delimiter)[1]

            if frame_header:
                return filter(lambda x: len(x) > 1, map(_processFrameHeader, content.split('#')))
            else:
                return filter(lambda x: len(x) > 0, content.split('\n'))
        except:
            return None

    return None
    


# Setup of the command-line arguments parser
parser = OptionParser("Usage: %prog [options] <coredump_file> [<root_folder 1>[:<label>] [<root_folder 2>[:<label>] [... <root_folder N>[:<label>]]]]", version="%prog 1.0")

parser.add_option("--tempfolder", action="store", default="", type="string",
                  dest="tempfolder", metavar="PATH",
                  help="Path to the folder were temporary files must be written (default: the same as the script)")

# Handling of the arguments
(options, args) = parser.parse_args()
if (len(args) < 1):
    parser.print_help()
    sys.exit(1)

coredump_file = args[0]

root_folders = {}
for root_folder in args[1:]:
    if root_folder.find(':') > 0:
        (folder, label) = root_folder.split(':')
        if not(label.endswith(os.path.sep)):
            label += os.path.sep
    else:
        folder = root_folder
        label = ''
    
    path = os.path.abspath(folder)
    root_folders[path + os.path.sep] = label
    
    realpath = os.path.realpath(path)
    root_folders[realpath + os.path.sep] = label
    
    finalpath = ''

    if os.path.isabs(folder):
        finalpath = os.path.sep

    if folder[-1] == os.path.sep:
        folder = folder[0:-1]

    for part in folder.split(os.path.sep):
        finalpath = os.path.join(finalpath, part)
        if os.path.islink(finalpath):
            finalpath = os.path.realpath(finalpath)

    root_folders[finalpath + os.path.sep] = label


command_files_folder = os.path.dirname(sys.argv[0])

# Check that the temp folder exists
if len(options.tempfolder) == 0:
    options.tempfolder = command_files_folder

if not(os.path.exists(options.tempfolder)):
    os.makedirs(options.tempfolder)

# Check that the core dump file exists
if not(os.path.exists(coredump_file)):
    print "ERROR - File not found: %s" % coredump_file
    sys.exit(1)

# Make sure we can read it
p = subprocess.Popen(['./coredump_rights_changer', coredump_file],
                     stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                     stderr=subprocess.PIPE, universal_newlines=True)
(output, errors) = p.communicate()

# Retrieve the stacktrace
output = runDebugger(coredump_file, os.path.join(command_files_folder, 'get_stacktrace.cmd'), 'STACK_TRACE', True)

if len(root_folders) > 0:
    converted_output = []
    for line in output:
        offset = line.find(' at ')
        filename = line[offset + 4:]

        offset = filename.find(':')
        if offset > 0:
            filename = filename[:offset]

        converted_output.append(line.replace(filename, os.path.realpath(filename)))

    stacktrace = []
    for root_folder, label in root_folders.items():
        filtered_stacktrace = filter(lambda x: x.find(root_folder) > 0, converted_output)
        converted_stacktrace = map(lambda x: x.replace(root_folder, label), filtered_stacktrace)
        stacktrace.extend(converted_stacktrace)

    if len(stacktrace) == 0:
        print "ERROR - The stacktrace doesn't contain anything from the root folders (%s)" % (', '.join(root_folders.keys()), )
        sys.exit(1)

    stacktrace.sort(cmp=cmp_stacktrace_entries)
else:
    stacktrace = output


# Retrieve the details of each frame
template_file = open(os.path.join(command_files_folder, 'get_stackframe.cmd.template'), 'r')
template = template_file.read()
template_file.close()

result = []
regex_frame_number = re.compile(u'^#(\d+)\s', flags=re.UNICODE)
regex_line_number = re.compile(u':(\d+)$', flags=re.UNICODE)
for line in stacktrace:
    # Ensure that the frame header has the correct format
    try:
        frame_index = int(regex_frame_number.search(line).group(1))
    except:
        print "ERROR - The following frame header doesn't have the correct format (missing frame index): %s" % line
        sys.exit(1)
    
    try:
        line_number = int(regex_line_number.search(line).group(1))
    except:
        print "ERROR - The following frame header doesn't have the correct format (missing line number): %s" % line
        sys.exit(1)
    
    content = template.replace('$FRAME', '%d' % frame_index)

    command_file = open(os.path.join(options.tempfolder, 'get_stackframe.cmd'), 'w')
    command_file.write(content)
    command_file.close()

    output = runDebugger(coredump_file, os.path.join(options.tempfolder, 'get_stackframe.cmd'), 'FRAME')

    result.append(line)

    # Ensure that the line number reported in the frame header is in the output
    if len(filter(lambda x: x.startswith('%d' % line_number), output)) == 1:
        result.extend(output)
    else:
        result.extend(['%d  (no source code to display)' % line_number])


# Report the stacktrace
if len(result) == 0:
    print """ERROR - Empty stacktrace

The output of the debugger is:
--------------------
%s
--------------------

The filtered stacktrace is:
--------------------
%s
--------------------
""" % ('\n'.join(output), '\n'.join(stacktrace))
    sys.exit(1)


print '\n'.join(result)

sys.exit(0)

