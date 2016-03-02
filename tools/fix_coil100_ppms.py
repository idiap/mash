#! /usr/bin/python

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


import struct
import sys
import os
import re

if len(sys.argv) != 3:
    print 'Usage: ' + sys.argv[0] + ' <original db path> <fixed db path>'
    exit(0)

# Retrieve the list of ppm files in the original db folder
files = os.listdir(sys.argv[1])
ppmsRegex = re.compile("^.+\.ppm$")
files = filter(ppmsRegex.search, files)

for filename in files:
    print filename
    
    inFile = open(os.path.join(sys.argv[1], filename), 'rb')
    outFile = open(os.path.join(sys.argv[2], filename), 'wb')

    outFile.write(inFile.readline())    # P6
    outFile.write(inFile.readline())    # Comment
    outFile.write(inFile.readline())    # Comment
    outFile.write(inFile.readline())    # Size

    line = inFile.readline()            # Max value
    maxvalue = int(line.split(' ')[0])
    outFile.write('255\n')

    for i in range(0, 128*128):
        rgb = inFile.read(6)
        (r, g, b) = struct.unpack('HHH', rgb)
        outFile.write(struct.pack('BBB', int(255 * (float(r) / maxvalue)),
                                         int(255 * (float(g) / maxvalue)),
                                         int(255 * (float(b) / maxvalue))))

    inFile.close()
    outFile.close()
