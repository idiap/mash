# Paths (when using the source package)
H################################################################################
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


EURISTICS_REPOSITORY_PATH  = 'repositories/upload-$INSTANCE.git'
HEURISTICS_CMAKE_PATH       = 'heuristics_cmake'
HEURISTICS_DEST_FOLDER      = 'compilation_server_heuristics/$INSTANCE'
BIN_PATH                    = '../build/bin'
HEURISTICS_BUILD_FOLDER     = 'build/$INSTANCE'
DATA_PATH                   = '../../data'
LOG_FOLDER                  = 'logs/$INSTANCE'

# Paths (when the Compilation Server is installed)
#HEURISTICS_REPOSITORY_PATH  = 'repositories/upload-$INSTANCE.git'
#HEURISTICS_CMAKE_PATH       = 'heuristics_cmake'
#HEURISTICS_DEST_FOLDER      = 'heuristics/$INSTANCE'
#BIN_PATH                    = '.'
#HEURISTICS_BUILD_FOLDER     = 'build/$INSTANCE'
#DATA_PATH                   = 'data'
#LOG_FOLDER                  = 'logs/$INSTANCE'

# Sandboxing
CORE_DUMP_TEMPLATE          = 'core'
SANDBOX_USERNAME            = ''
SANDBOX_JAILDIR             = 'jail/$INSTANCE'
SANDBOX_SCRIPTSDIR          = '../sandbox'
# SANDBOX_SCRIPTSDIR          = '.'
SANDBOX_TEMPDIR             = 'temp/$INSTANCE'
