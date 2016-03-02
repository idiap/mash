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



def testTransmission(client, command):
    if not(client.sendCommand(command)):
        print "Failed to send command '%s' to the server" % command
        return None

    response = client.waitResponse()
    if response is None:
        print "Failed to wait for response to command '%s' from the server" % command.toString()
        return None

    CHECK_EQUAL(command.toString(), response.toString());

    return response



# Parameters handling
if (len(sys.argv) > 2) or ((len(sys.argv) == 2) and (sys.argv[1] == '--help')):
    print "Usage: %s [MODULES_PATH]" % sys.argv[0]
    sys.exit(-1)

if len(sys.argv) == 2:
    sys.path.append(sys.argv[1])

from pymash import Client
from pymash import Message
    

# Create the client and connect to the server
client = Client()
if not(client.connect('127.0.0.1', 10000)):
    print 'Failed to connect to the server'
    sys.exit(-1)


response = testTransmission(client, Message("NO_ARGUMENT"))
if response is None:
    sys.exit(-1)


commandArgs = ["http://something.com"]
response = testTransmission(client, Message("ONE_STRING_ARGUMENT", commandArgs))
if response is None:
    sys.exit(-1)

CHECK_EQUAL(commandArgs[0], response.parameters[0]);


commandArgs = [100]
response = testTransmission(client, Message("ONE_INT_ARGUMENT", commandArgs))
if response is None:
    sys.exit(-1)

CHECK_EQUAL(commandArgs[0], int(response.parameters[0]));


commandArgs = [0.8]
response = testTransmission(client, Message("ONE_FLOAT_ARGUMENT", commandArgs))
if response is None:
    sys.exit(-1)

CHECK_EQUAL(commandArgs[0], float(response.parameters[0]));


commandArgs = ["http://something.com", 100, 0.8]
response = testTransmission(client, Message("THREE_ARGUMENTS", commandArgs))
if response is None:
    sys.exit(-1)

CHECK_EQUAL(commandArgs[0], response.parameters[0]);
CHECK_EQUAL(commandArgs[1], int(response.parameters[1]));
CHECK_EQUAL(commandArgs[2], float(response.parameters[2]));


commandArgs = ["Hello World!"]
response = testTransmission(client, Message("ONE_QUOTED_STRING_ARGUMENT", commandArgs))
if response is None:
    sys.exit(-1)

CHECK_EQUAL(commandArgs[0], response.parameters[0]);


commandArgs = ["Escaped 'Hello world!' string"]
response = testTransmission(client, Message("ONE_QUOTED_STRING_WITH_ESCAPING_ARGUMENT", commandArgs))
if response is None:
    sys.exit(-1)

CHECK_EQUAL(commandArgs[0], response.parameters[0]);


commandArgs = ["Long string on\nseveral lines"]
response = testTransmission(client, Message("ONE_QUOTED_STRING_WITH_SEVERAL_LINES_ARGUMENT", commandArgs))
if response is None:
    sys.exit(-1)

CHECK_EQUAL(commandArgs[0], response.parameters[0]);


client.close()
