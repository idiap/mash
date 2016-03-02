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


def CHECK_EQUAL(expected, actual):
    if expected != actual:
        print "Failure: expected %s, got %s" % (str(expected), str(actual))
        sys.exit(-1)


# Parameters handling
if (len(sys.argv) > 2) or ((len(sys.argv) == 2) and (sys.argv[1] == '--help')):
    print "Usage: %s [MODULES_PATH]" % sys.argv[0]
    sys.exit(-1)

if len(sys.argv) == 2:
    sys.path.append(sys.argv[1])

from pymash import Client, Message

sys.path.append(os.path.dirname(sys.argv[0]))

import binary_data
    

# Create the client and connect to the server
client = Client()
if not(client.connect('127.0.0.1', 10000)):
    print 'Failed to connect to the server'
    sys.exit(-1)


if not(client.sendCommand('SEND_DATA')):
    print "Failed to send command 'SEND_DATA' to the server"
    sys.exit(-1)

response = client.waitResponse()
if response is None:
    print "Failed to wait for response to command 'SEND_DATA' from the server"
    sys.exit(-1)

CHECK_EQUAL('DATA', response.name)
CHECK_EQUAL(1, len(response.parameters))

size = int(response.parameters[0])
CHECK_EQUAL(binary_data.DATA_size, size)

data = client.waitData(size)

if data is None:
    print "Failed to wait for %d bytes of data from the server" % size
    sys.exit(-1)

for i in range(0, binary_data.DATA_size):
    CHECK_EQUAL(binary_data.DATA[i], ord(data[i]))

result = client.sendData(data)
if response is None:
    print "Failed to send data to the server"
    sys.exit(-1)

response = client.waitResponse()
if response is None:
    print "Failed to wait for message 'TEST_OK' from the server"
    sys.exit(-1)

CHECK_EQUAL('TEST_OK', response.name)
CHECK_EQUAL(None, response.parameters)

client.close()
