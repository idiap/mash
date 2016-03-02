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
import glob
import os

#-------------------------------------------------------------------------------
# Provides informations about the 'MNIST' database
#-------------------------------------------------------------------------------
class db_mnist(Database):

    NB_IMAGES       = 70000
    NB_LABELS       = 10
    IMAGES_WIDTH    = 29
    IMAGES_HEIGHT   = 29

    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param config     Configuration of the database
    #---------------------------------------------------------------------------
    def __init__(self, config):
        super(db_mnist, self).__init__(config)

        self.nb_training_images_per_label = []
        for i in range(0, db_mnist.NB_LABELS):
            files = glob.glob(os.path.join(self._setting('root_path', ''), 'train', '%d' % i, '*.png'))
            self.nb_training_images_per_label.append(len(files))

        self.nb_test_images_per_label = []
        for i in range(0, db_mnist.NB_LABELS):
            files = glob.glob(os.path.join(self._setting('root_path', ''), 'test', '%d' % i, '*.png'))
            self.nb_test_images_per_label.append(len(files))

    #---------------------------------------------------------------------------
    # Returns the number of images in the database
    #---------------------------------------------------------------------------
    def imagesCount(self):
        return db_mnist.NB_IMAGES

    #---------------------------------------------------------------------------
    # Returns the number of objects in the database
    #---------------------------------------------------------------------------
    def objectsCount(self):
        return db_mnist.NB_IMAGES

    #---------------------------------------------------------------------------
    # Returns the number of labels in the database
    #---------------------------------------------------------------------------
    def labelsCount(self):
        return db_mnist.NB_LABELS

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
        (label, offset, image_set) = self._imageLabel(index)
        return [Object(label, 0, 0, db_mnist.IMAGES_WIDTH - 1, db_mnist.IMAGES_HEIGHT - 1)]

    #---------------------------------------------------------------------------
    # Returns the name of an image file (relative to the root path where the
    # database is stored)
    #
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def imageName(self, index):
        (label, offset, image_set) = self._imageLabel(index)
        if label is None:
            return None

        if image_set == Database.TRAINING_SET:
            return 'train/%d/%04d.png' % (label, index - offset)
        else:
            return 'test/%d/%04d.png' % (label, index - offset)

    #---------------------------------------------------------------------------
    # Returns the size of an image
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The size as a tuple: (width, height)
    #---------------------------------------------------------------------------
    def imageSize(self, index):
        return (db_mnist.IMAGES_WIDTH, db_mnist.IMAGES_HEIGHT)

    #---------------------------------------------------------------------------
    # Returns the set (training or test) an image is part of
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The set, or None if not supported
    #---------------------------------------------------------------------------
    def imageSet(self, index):
        (label, offset, image_set) = self._imageLabel(index)
        return image_set

    #---------------------------------------------------------------------------
    # Returns the preferred image size of the database
    #
    # @return   The size as a tuple: (width, height), or None if not supported
    #---------------------------------------------------------------------------
    def preferredImageSize(self):
        return (db_mnist.IMAGES_WIDTH, db_mnist.IMAGES_HEIGHT)

    #---------------------------------------------------------------------------
    # Returns the preferred ROI size of the database
    #
    # @return   The size, or None if not supported
    #---------------------------------------------------------------------------
    def preferredRoiSize(self):
        return db_mnist.IMAGES_WIDTH

    def _imageLabel(self, index):
        if index >= self.imagesCount():
            return (None, None, None)

        label = 0
        sum = 0
        image_set = Database.TRAINING_SET
        while (label < len(self.nb_training_images_per_label)) and (index >= sum + self.nb_training_images_per_label[label]):
            sum += self.nb_training_images_per_label[label]
            label += 1

        if label == len(self.nb_training_images_per_label):
            label = 0
            image_set = Database.TEST_SET
            while (label < len(self.nb_test_images_per_label)) and (index >= sum + self.nb_test_images_per_label[label]):
                sum += self.nb_test_images_per_label[label]
                label += 1

        if label == len(self.nb_test_images_per_label):
            label = -1

        return (label, sum, image_set)
