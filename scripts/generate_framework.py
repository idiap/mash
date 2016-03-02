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
ARCHIVE_PREFIX  = 'mash'


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
packager.copyFiles('.', ['CMakeLists.txt', 'README'])
packager.copyTree('cmake')
packager.copyTree('data')
packager.copyTree('dependencies')
packager.copyFiles('heuristics', ['CMakeLists.txt', 'heuristic.def', 'template.cpp'], dest_path='heuristics')
packager.copyTree('heuristics/examples')
packager.copyTree('heuristics/unittests')
packager.copyTree('heuristics/upload')
packager.copyFiles('classifiers', ['CMakeLists.txt', 'template.cpp'], dest_path='classifiers')
packager.copyTree('classifiers/examples')
packager.copyTree('classifiers/mash')
packager.copyTree('classifiers/unittests')
packager.copyFiles('goalplanners', ['CMakeLists.txt', 'template.cpp'], dest_path='goalplanners')
packager.copyTree('goalplanners/examples')
packager.copyTree('goalplanners/libraries')
packager.copyTree('goalplanners/unittests')
packager.copyFiles('instruments', ['CMakeLists.txt', 'template.cpp'], dest_path='instruments')
packager.copyTree('instruments/mash')
packager.copyTree('instruments/unittests')
packager.copyTree('sdk')
packager.copyTree('experiment-server', ['*.pyc', 'settings_*.txt'])
packager.copyFiles('experiment-server', ['settings_classification_example.txt', 'settings_goalplanning_example.txt', 'settings_detection_example.txt'], dest_path='experiment-server')
packager.copyTree('compilation-server', ['*.pyc', 'config.py', 'pymash', 'repositories', 'logs'])
packager.copyTree('application-servers', ['*.pyc', '*.data', 'config.py', 'pymash', 'logs', 'config.txt'])
packager.copyTree('tests', ['compilation-server-config.py'])
packager.copyFiles('tools', ['CMakeLists.txt', 'test_client.py', 'interactive_client.py', 'keys_maze.txt', 'keys_simulation.txt'])
packager.copyTree('scripts', ['logs'])
packager.copyTree('unittests')
packager.copyTree('mash')
packager.copyTree('mash-appserver')
packager.copyTree('mash-classification')
packager.copyTree('mash-goalplanning')
packager.copyTree('mash-instrumentation')
packager.copyTree('mash-network')
packager.copyTree('mash-sandboxing')
packager.copyTree('mash-utils')
packager.copyTree('pymash', ['*.pyc'])
packager.copyTree('sandbox', ['*.pyc'])


# Create the archive
packager.createTarGz()
