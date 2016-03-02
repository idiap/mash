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


databases = {}

# Database 'coil-100' settings
databases['coil-100'] = {
    'enabled': True,
    'class': 'db_coil100',
    'url_prefix': 'http://path/to/the/database/',
    'root_path': '/path/to/the/database/',
}

# Database 'caltech-256' settings
databases['caltech-256'] = {
    'enabled': True,
    'class': 'db_caltech256',
    'url_prefix': 'http://path/to/the/database/',
    'root_path': '/path/to/the/database/',
    'original_images': 'original',
    'preprocessed_images': 'preprocessed',
}

# Database 'mnist' settings
databases['mnist'] = {
    'enabled': True,
    'class': 'db_mnist',
    'url_prefix': 'http://path/to/the/database/',
    'root_path': '/path/to/the/database/',
}

# Database 'minimnist' settings
databases['mini-mnist'] = {
    'enabled': True,
    'class': 'db_minimnist',
    'url_prefix': 'http://path/to/the/database/',
    'root_path': '/path/to/the/database/',
}

# Database 'mugs' settings
databases['mugs'] = {
    'enabled': True,
    'class': 'db_mugs',
    'url_prefix': 'http://path/to/the/database/',
    'root_path': '/path/to/the/database/',
}

# Database 'cifar-10' settings
databases['cifar-10'] = {
    'enabled': True,
    'class': 'db_cifar10',
    'url_prefix': 'http://path/to/the/database/',
    'root_path': '/path/to/the/database/',
}

# Database 'inria-person' settings
databases['inria-person'] = {
    'enabled': True,
    'class': 'db_inriaperson',
    'url_prefix': 'http://path/to/the/database/',
    'root_path': '/path/to/the/database/',
}
