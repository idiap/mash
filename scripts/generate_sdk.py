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
from mash import Packager


# Constants
DISTRIB_FOLDER  = 'distrib'
ARCHIVE_PREFIX  = 'mash_sdk'


# Process the command-line arguments
if len(sys.argv) != 2:
    print "Usage: %s <version>" % sys.argv[0]
    print
    sys.exit(-1)


# Go to the working directory
directory = os.path.abspath(os.path.dirname(sys.argv[0]))
os.chdir(os.path.join(directory, '..'))


# Create the packager
packager = Packager(os.getcwd(), os.path.join(os.getcwd(), DISTRIB_FOLDER), ARCHIVE_PREFIX + '_' + sys.argv[1])


# Copy the needed files
packager.copyFile('.', 'CMakeLists.sdk', 'CMakeLists.txt')
packager.copyTree('cmake')
packager.copyFiles('data', ['*.ppm', '*.jpg', 'CREDITS.TXT'])
packager.copyFiles('dependencies', 'CMakeLists.txt')
packager.copyTree('dependencies/include')
packager.copyTree('dependencies/FreeImage')
packager.copyFiles('heuristics', ['CMakeLists.txt', 'template.cpp', 'heuristic.def'])
packager.copyTree('heuristics/examples')
packager.copyTree('sdk', ignore_patterns=['README.txt', 'LICENSE.txt'])
packager.copyFiles('sdk', ['README.txt', 'LICENSE.txt'], dest_path='.')
packager.copyFiles('mash', ['CMakeLists.txt',
                            'heuristic.h',
                            'heuristics_manager.*',
                            'dynlibs_manager.*',
                            'dynlibs_manager_delegate_interface.h',
                            'image.*',
                            'imageutils.*',
                           ])
packager.copyFiles('mash-utils', ['CMakeLists.txt',
                                  'declarations.h',
                                  'platform.h',
                                  'errors.*',
                                  'stringutils.*',
                                 ])
packager.copyFiles('scripts', 'create_heuristic.py', '.')


# Create the archives
packager.createTarGz()
packager.createZip()
