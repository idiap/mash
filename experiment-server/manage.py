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


################################################################################
#
# This script is used by the 'Experiment Server' to perform the following
# operations on the heuristics:
#   - cloning of the GIT repository
#   - compilation
#
################################################################################

import sys
import os
import subprocess
from optparse import OptionParser


################################### CONSTANTS ##################################

# The actions that can be performed
ACTION_CLONE        = 1
ACTION_COMPILATION  = 2


################################## FUNCTIONS ###################################

#-------------------------------------------------------------------------------
# Clone a GIT repository
#
# @param repository_url     URL of the GIT repository to clone
# @param destination        Destination folder
# @param quiet              Don't write non-error messages to the console output
#-------------------------------------------------------------------------------
def cloneRepository(repository_url, destination, quiet=False):

    # Test if the destination directory already exists
    if os.path.exists(destination):

        current_dir = os.getcwd()
        os.chdir(destination)
        
        # Test if it was cloned from the same URL
        p = subprocess.Popen(['git', 'config', '--get', 'remote.origin.url'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if p.wait() == 0:
            if p.stdout.read()[0:-1] == repository_url:
                # Only update the working copy
                if not(quiet):
                    print '  The repository is already cloned in that directory, updating...'

                p = subprocess.Popen(['git', 'pull'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                if p.wait() == 0:
                    sys.exit(0)

                if not(quiet):
                    print '  Failed to update!'
                    print p.stdout.read()

        # Remove the destination folder
        if not(quiet):
            print '  Removing the destination folder...'
        os.chdir('..')
        os.system('rm -rf "%s"' % os.path.basename(destination))

        os.chdir(current_dir)

    # Determine the parent path of the destination
    full_path = os.path.abspath(destination)
    parent_dir = full_path[0:-len(os.path.basename(full_path))]

    # Create the parent folder if necessary
    if not(os.path.exists(parent_dir)):
        os.makedirs(parent_dir)

    # Clone the repository
    if not(quiet):
        print '  Cloning of the repository...'
    current_dir = os.getcwd()
    os.chdir(parent_dir)
    p = subprocess.Popen(['git', 'clone', '%s' % repository_url, '%s' % os.path.basename(full_path)],
                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    ret = p.wait()
    if (ret != 0) and not(quiet):
        print p.stdout.read()
    os.chdir(current_dir)
    
    sys.exit(ret)


#-------------------------------------------------------------------------------
# Compile a heuristic
#
# @param src_file           Path to the C++ source file that implement the
#                           heuristic
# @param heuristic_name     Name of the heuristic
# @param destination        Destination folder of the heuristic
# @param build_dir          Build directory
# @param libraries_dir      Directory in which the MASH libraries are located
# @param quiet              Don't write non-error messages to the console output
#-------------------------------------------------------------------------------
def compileHeuristic(src_file, heuristic_name, destination, build_dir,
                    libraries_dir, quiet=False):
    
    if heuristic_name.endswith('/1'):
        heuristic_name = heuristic_name[:-2]
    
    # Check that the source file exists
    src_file = os.path.abspath(src_file)
    if not(os.path.exists(src_file)):
        sys.exit(1)

    # Go into the folder containing the CMake-related files to compile the heuristics
    current_dir = os.getcwd()
    script_dir = os.path.abspath(sys.argv[0])[0:-len(os.path.basename(sys.argv[0]))-1]
    
    # Remove the build dir of this heuristic if one exists
    full_build_dir = os.path.join(build_dir, heuristic_name)
    if os.path.exists(full_build_dir):
        os.system('rm -rf %s' % full_build_dir)

    # Create the build dir
    os.makedirs(full_build_dir)
    os.chdir(full_build_dir)
    
    # Generate the makefiles
    p = subprocess.Popen(['cmake', '-DSRC_FILE=%s' % src_file, '-DFULL_HEURISTIC_NAME=%s' % heuristic_name,
                          '-DHEURISTICS_BUILD_DIR=%s' % destination, '-DMASH_BIN_DIR=%s' % libraries_dir,
                          os.path.join(script_dir, 'heuristics_cmake')],
                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    ret = p.wait()
    if ret == 0:
        # Compile the heuristic
        p = subprocess.Popen('make', stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        ret = p.wait()
        if (ret != 0) and not(quiet):
            print p.stdout.read()
    elif not(quiet):
        print p.stdout.read()

    # Fix: sys.exit belows doesn't like return code > 127, so always use 1 to signal an error
    if ret != 0:
        ret = 1

    os.chdir(current_dir)

    sys.exit(ret)


##################################### MAIN #####################################

if __name__ == "__main__":

    # Setup of the command-line arguments parser
    usage = """Usage: %prog [options] --clone REPOSITORY_URL DESTINATION
       %prog [options] --compile SRC_FILE HEURISTIC_NAME DESTINATION BUILD_DIR LIBRARIES_DIR"""
    parser = OptionParser(usage, version="%prog 1.0")
    parser.add_option("-q", "--quiet", action="store_true", default=False,
                      dest="quiet", help="Don't write non-error messages to the console output")
    parser.add_option("--clone", action="store_const", const=ACTION_CLONE,
                      dest="action", help="Clone the GIT repository at REPOSITORY_URL into the folder DESTINATION")
    parser.add_option("--compile", action="store_const", const=ACTION_COMPILATION,
                      dest="action", help="Compile the heuristic implemented in SRC_FILE, name it HEURISTIC_NAME, " \
                      "save it in the folder DESTINATION and use the BUILD_DIR directory to build it")

    # Handling of the arguments
    (options, args) = parser.parse_args()
    if options.action is None:
        parser.print_help()
        sys.exit(0)
    
    # Action: Clone
    if options.action == ACTION_CLONE:
        if len(args) != 2:
            parser.error("Incorrect number of arguments")
        cloneRepository(args[0], args[1], options.quiet)
        
    # Action: Compilation
    elif options.action == ACTION_COMPILATION:
        if len(args) != 5:
            parser.error("Incorrect number of arguments")
        compileHeuristic(args[0], args[1], args[2], args[3], args[4], options.quiet)

    parser.error("Unknown action")
