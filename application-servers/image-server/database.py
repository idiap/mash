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


import os


#-------------------------------------------------------------------------------
# Custom exception class
#-------------------------------------------------------------------------------
class DatabaseException(Exception):
    
    def __init__(self, message):
        super(Exception, self).__init__(message)
        

#-------------------------------------------------------------------------------
# Contains all the needed informations about one of the objects found in an
# image
#-------------------------------------------------------------------------------
class Object:

    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param label      Label of the object (0 .. labelsCount() - 1)
    # @param x1, y1     Top-left coordinates of the object in the image
    # @param x2, y2     Bottom-right coordinates of the object in the image
    #---------------------------------------------------------------------------
    def __init__(self, label, x1, y1, x2, y2):
        self.label = label
        self.topLeft = (x1, y1)
        self.bottomRight = (x2, y2)


#-------------------------------------------------------------------------------
# Base class for all the database-specific classes
#-------------------------------------------------------------------------------
class Database(object):

    ############################# CLASS ATTRIBUTES #############################
    
    constructors = {}
    
    TRAINING_SET = 0
    TEST_SET = 1
    

    ############################### CONSTRUCTION ###############################

    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param config     Configuration of the database
    #---------------------------------------------------------------------------
    def __init__(self, config):
        self.config = config
        self.use_preprocessed_images = False
    

    ########################### METHODS TO IMPLEMENT ###########################

    #---------------------------------------------------------------------------
    # Returns the number of images in the database
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def imagesCount(self):
        raise NotImplemented('Each database class must implement the imagesCount() method')

    #---------------------------------------------------------------------------
    # Returns the number of objects in the database
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def objectsCount(self):
        raise NotImplemented('Each database class must implement the objectsCount() method')

    #---------------------------------------------------------------------------
    # Returns the number of labels in the database
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def labelsCount(self):
        raise NotImplemented('Each database class must implement the labelsCount() method')

    #---------------------------------------------------------------------------
    # Returns the list of the names of the labels, or None if the database
    # doesn't provide that information
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def labelNamesList(self):
        raise NotImplemented('Each database class must implement the labelsList() method')

    #---------------------------------------------------------------------------
    # Returns a list of the objects in an image
    #
    # The list must contain instances of the 'Object' class
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def objectsInImage(self, index):
        raise NotImplemented('Each database class must implement the imageName() method')

    #---------------------------------------------------------------------------
    # Returns the name of an image file (relative to the root path where the
    # database is stored)
    #
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def imageName(self, index):
        raise NotImplemented('Each database class must implement the imageName() method')

    #---------------------------------------------------------------------------
    # Returns the size of an image
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The size as a tuple: (width, height)
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def imageSize(self, index):
        raise NotImplemented('Each database class must implement the imageSize() method')

    #---------------------------------------------------------------------------
    # Returns the set (training or test) an image is part of
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The set, or None if not supported
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES FOR WHICH IT MAKES SENSE
    #---------------------------------------------------------------------------
    def imageSet(self, index):
        return None

    #---------------------------------------------------------------------------
    # Returns the preferred image size of the database
    #
    # @return   The size as a tuple: (width, height), or None if not supported
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def preferredImageSize(self):
        raise NotImplemented('Each database class must implement the preferredImageSize() method')

    #---------------------------------------------------------------------------
    # Returns the preferred ROI size of the database
    #
    # @return   The size, or None if not supported
    #
    # MUST BE IMPLEMENTED BY THE SUBCLASSES
    #---------------------------------------------------------------------------
    def preferredRoiSize(self):
        raise NotImplemented('Each database class must implement the preferredSize() method')


    ################################## METHODS #################################

    #---------------------------------------------------------------------------
    # Enable/disable the use of the preprocessed images
    #---------------------------------------------------------------------------
    def usePreprocessedImages(self, enabled):
        if self._setting('original_images', '') == self._setting('preprocessed_images', ''):
            return False
        
        self.use_preprocessed_images = enabled
        
        return True

    #---------------------------------------------------------------------------
    # Returns the URL prefix of the database
    #---------------------------------------------------------------------------
    def urlPrefix(self):
        url = self._setting('url_prefix', '')
        if (len(url) > 0) and not(url.endswith('/')):
            url += '/'

        if url.startswith('file://'):
            return url[7:]

        if url.find('://') > 0:
            return url

        return os.path.abspath(url)

    #---------------------------------------------------------------------------
    # Returns the fully qualified name of an image
    #
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def fullImageName(self, index):
        folder = ''
        
        if self.use_preprocessed_images:
            folder = self._setting('preprocessed_images', '')
        else:
            folder = self._setting('original_images', '')

        if (len(folder) > 0) and not(folder.endswith('/')):
            folder += '/'

        return folder + self.imageName(index)

    #---------------------------------------------------------------------------
    # Returns the path to an image
    #
    # @param image_name     Image name
    #---------------------------------------------------------------------------
    def path(self, image_name):
        return os.path.abspath(self._setting('root_path', '') + image_name)

    #---------------------------------------------------------------------------
    # Returns the value of one of the settings of the database
    #
    # @param name       Setting name
    # @param default    Default value (returned if no setting with the provided
    #                   name is found)
    #---------------------------------------------------------------------------
    def _setting(self, name, default):
        if self.config.has_key(name):
            return self.config[name]
        return default
