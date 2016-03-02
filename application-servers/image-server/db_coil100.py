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


from database import Database, Object

#-------------------------------------------------------------------------------
# Provides informations about the 'coil-100' database
#-------------------------------------------------------------------------------
class db_coil100(Database):
    
    ##### Constants #####
    
    NB_IMAGES_PER_LABEL = 72
    NB_LABELS           = 100
    IMAGES_WIDTH        = 127
    IMAGES_HEIGHT       = 127
    

    #---------------------------------------------------------------------------
    # Returns the number of images in the database
    #---------------------------------------------------------------------------
    def imagesCount(self):
        return db_coil100.NB_LABELS * db_coil100.NB_IMAGES_PER_LABEL

    #---------------------------------------------------------------------------
    # Returns the number of objects in the database
    #---------------------------------------------------------------------------
    def objectsCount(self):
        return db_coil100.NB_LABELS * db_coil100.NB_IMAGES_PER_LABEL

    #---------------------------------------------------------------------------
    # Returns the number of labels in the database
    #---------------------------------------------------------------------------
    def labelsCount(self):
        return db_coil100.NB_LABELS

    #---------------------------------------------------------------------------
    # Returns the list of the names of the labels, or None if the database
    # doesn't provide that information
    #---------------------------------------------------------------------------
    def labelNamesList(self):
        return None

    #---------------------------------------------------------------------------
    # Returns a list of the objects in an image
    #
    # The list must contain instances of the 'Object' class
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def objectsInImage(self, index):
        label = index / db_coil100.NB_IMAGES_PER_LABEL
        
        return [Object(label, 0, 0, db_coil100.IMAGES_WIDTH - 1, db_coil100.IMAGES_HEIGHT - 1)]

    #---------------------------------------------------------------------------
    # Returns the name of an image file (relative to the root path where the
    # database is stored)
    #
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def imageName(self, index):
        if index >= self.imagesCount():
            return None

        label = (index / db_coil100.NB_IMAGES_PER_LABEL) + 1
        pose = index % db_coil100.NB_IMAGES_PER_LABEL

        return 'obj%d__%d.png' % (label, pose * 5)

    #---------------------------------------------------------------------------
    # Returns the size of an image
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The size as a tuple: (width, height)
    #---------------------------------------------------------------------------
    def imageSize(self, index):
        return (db_coil100.IMAGES_WIDTH, db_coil100.IMAGES_HEIGHT)

    #---------------------------------------------------------------------------
    # Returns the preferred image size of the database
    #
    # @return   The size as a tuple: (width, height), or None if not supported
    #---------------------------------------------------------------------------
    def preferredImageSize(self):
        return (db_coil100.IMAGES_WIDTH, db_coil100.IMAGES_HEIGHT)

    #---------------------------------------------------------------------------
    # Returns the preferred ROI size of the database
    #
    # @return   The size, or None if not supported
    #---------------------------------------------------------------------------
    def preferredRoiSize(self):
        return db_coil100.IMAGES_WIDTH
