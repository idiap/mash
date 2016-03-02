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
import subprocess
import signal
import time


# Parameters handling
if len(sys.argv) != 4:
    print "Usage: %s PATH_TO_SERVER_APPLICATION PATH_TO_CLIENT_APPLICATION WORKING_DIRECTORY" % sys.argv[0]
    sys.exit(-1)

server_path = sys.argv[1]
client_path = sys.argv[2]
cwd = sys.argv[3]

# Start the server application
command = server_path.split()
command.append('5')
server = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd)

time.sleep(2)

# Start the client application
command = client_path.split()
command.append('5')
client = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd)

if client.wait() != 0:
    print "Client stdout: " + client.stdout.read()
    print "Server stdout: " + server.stdout.read()
    os.kill(server.pid, signal.SIGKILL)
    sys.exit(-1)

os.kill(server.pid, signal.SIGKILL)
