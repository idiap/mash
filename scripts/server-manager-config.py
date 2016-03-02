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


host = '127.0.0.1'
port = 20000


server_types = {
    'compilox':     { 'path':               './compilation-server.py',
                      'user':               None,
                      'cwd':                'compilation-server',
                      '--host':             '127.0.0.1',
                      '--instance':         '$(name)',
                    },

    'experimentix': { 'path':               './experiment-server',
                      'user':               None,
                      'cwd':                'build/bin',
                      '--host':             '127.0.0.1',
                      '--scriptsdir':       '../../experiment-server',
                      '--repository':       'repositories/heuristics-$(name).git',
                      '--heuristicsdir':    'heuristics_live/$(name)',
                      '--builddir':         'build/$(name)',
                      '--logfolder':        'logs/experiment-server/$(name)',
                    },

    'imagix':       { 'path':               './image-server.py',
                      'user':               None,
                      'cwd':                'application-servers/image-server',
                      '--host':             '127.0.0.1',
                      '--instance':         '$(name)',
                    },

    'mazox':        { 'path':               './maze-server',
                      'user':               None,
                      'cwd':                'build/bin',
                      '--host':             '127.0.0.1',
                      '--logfolder':        'logs/maze-server/$(name)',
                    },

    'scheduler':    { 'path':               './experiment-scheduler.py',
                      'user':               None,
                      'cwd':                '../mash-web/experiment-scheduler',
                      '--host':             '127.0.0.1',
                      '--port':             20000,
                    },
}


servers = [

    # Compilation Servers
    { 'name':       'compilox1',
      '--port':     20101,
    },

    { 'name':       'compilox2',
      '--port':     20102,
    },

    # Experiment Servers
    { 'name':       'experimentix1',
      '--port':     20201,
    },

    { 'name':       'experimentix2',
      '--port':     20202,
    },

    # Image Servers
    { 'name':       'imagix1',
      '--port':     20301,
    },

    { 'name':       'imagix2',
      '--port':     20302,
    },

    # Maze Servers
    { 'name':       'mazox1',
      '--port':     20401,
    },

    { 'name':       'mazox2',
      '--port':     20402,
    },

    # Experiment Scheduler
    { 'name':       'scheduler',
    },
]
