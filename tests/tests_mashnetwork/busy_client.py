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
import time


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
if (len(sys.argv) > 3) or ((len(sys.argv) == 2) and (sys.argv[1] == '--help')):
    print "Usage: %s [MODULES_PATH] NB_MAX_CLIENTS" % sys.argv[0]
    sys.exit(-1)

if len(sys.argv) == 3:
    sys.path.append(sys.argv[1])
    nb_max_clients = int(sys.argv[2])
else:
    nb_max_clients = int(sys.argv[1])
    
from pymash import Client
from pymash import Message


clients = []


# Create the clients and connect to the server
for i in range(0, nb_max_clients + 5):
    client = Client()
    if not(client.connect('127.0.0.1', 10000)):
        print 'Failed to connect to the server (%d)' % i
        sys.exit(-1)
    clients.append(client)

time.sleep(1)


# Check that nb_max_clients clients are OK and 5 aren't
nb_ok = 0
to_remove = []
for client in clients:
    if not(client.sendCommand('STATUS')):
        print "Failed to send command 'STATUS' to the server"
        sys.exit(-1)

    response = client.waitResponse()
    if response is None:
        print "Failed to wait for response to command 'STATUS' from the server"
        sys.exit(-1)

    print response.toString()

    if response.name == 'BUSY':
        to_remove.append(client)
    else:
        nb_ok += 1

if nb_ok != nb_max_clients:
    print "%d/%d clients are successfully connected" % (nb_ok, nb_max_clients)
    sys.exit(-1)


# Close the busy clients
for client in to_remove:
    client.close()
    clients.remove(client)


# Close one connected client
client = clients[0]
clients = clients[1:]
client.close()

time.sleep(2)

# Connect a new client
client = Client()
if not(client.connect('127.0.0.1', 10000)):
    print 'Failed to connect to the server'
    sys.exit(-1)
clients.append(client)


# Check that it can communicate
response = testTransmission(client, Message("STATUS"))
if response is None:
    sys.exit(-1)


# Close everything
for client in clients:
    client.close()
