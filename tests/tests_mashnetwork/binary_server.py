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


# Parameters handling
if (len(sys.argv) > 3) or ((len(sys.argv) == 2) and (sys.argv[1] == '--help')):
    print "Usage: %s [MODULES_PATH [NB_MAX_CLIENT]]" % sys.argv[0]
    sys.exit(-1)

if len(sys.argv) == 3:
    sys.path.append(sys.argv[1])
    nb_max_clients = int(sys.argv[2])
elif len(sys.argv) == 2:
    sys.path.append(sys.argv[1])
    nb_max_clients = 0
else:
    nb_max_clients = 0

from pymash import ServerListener, Server, Message

sys.path.append(os.path.dirname(sys.argv[0]))

import binary_data
    

class BinaryListener(ServerListener):

    def handleCommand(self, command):
        if command.name != "SEND_DATA":
            if not(self.sendResponse("UNKNOWN_COMMAND")):
                return ServerListener.ACTION_CLOSE_CONNECTION
        else:
            if not(self.sendResponse(Message("DATA", [binary_data.DATA_size]))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            if not(self.sendData(''.join(map(lambda x: chr(x), binary_data.DATA)))):
                return ServerListener.ACTION_CLOSE_CONNECTION

            data = self.waitData(binary_data.DATA_size)

            if data is None:
                return ServerListener.ACTION_CLOSE_CONNECTION

            for i in range(0, binary_data.DATA_size):
                if binary_data.DATA[i] != ord(data[i]):
                    return ServerListener.ACTION_CLOSE_CONNECTION

            if not(self.sendResponse("TEST_OK")):
                return ServerListener.ACTION_CLOSE_CONNECTION

        return ServerListener.ACTION_NONE


server = Server(nb_max_clients)
server.run('127.0.0.1', 10000, BinaryListener)
