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


class Batch:
    
    def __init__(self):
        self.image_set           = Database.TRAINING_SET
        self.path                = None
        self.fullpath            = None
        self.nb_images_per_label = []
        self.nb_images           = 0
        self.start_index         = 0



#-------------------------------------------------------------------------------
# Provides informations about the 'MNIST' database
#-------------------------------------------------------------------------------
class db_cifar10(Database):

    NB_LABELS       = 10
    IMAGES_WIDTH    = 33
    IMAGES_HEIGHT   = 33

    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param config     Configuration of the database
    #---------------------------------------------------------------------------
    def __init__(self, config):
        super(db_cifar10, self).__init__(config)

        self.batches = []
        
        batches = [ os.path.join(self._setting('root_path', ''), 'data_batch_%d' % i) for i in range(1, 5) ]
        batches.append(os.path.join(self._setting('root_path', ''), 'test_batch'))
        for path in batches:
            batch = Batch()
            batch.fullpath = path
            batch.path = os.path.basename(path)
            
            if batch.path == 'test_batch':
                batch.image_set = Database.TEST_SET

            for i in range(0, db_cifar10.NB_LABELS):
                files = glob.glob(os.path.join(batch.fullpath, '%d' % i, '*.png'))
                batch.nb_images_per_label.append(len(files))
            
            batch.nb_images = sum(batch.nb_images_per_label)

            if len(self.batches) > 0:
                previous = self.batches[-1]
                batch.start_index = previous.start_index + previous.nb_images

            self.batches.append(batch)

        last = self.batches[-1]
        self.NB_IMAGES = last.start_index + last.nb_images
        
    #---------------------------------------------------------------------------
    # Returns the number of images in the database
    #---------------------------------------------------------------------------
    def imagesCount(self):
        return self.NB_IMAGES

    #---------------------------------------------------------------------------
    # Returns the number of objects in the database
    #---------------------------------------------------------------------------
    def objectsCount(self):
        return self.NB_IMAGES

    #---------------------------------------------------------------------------
    # Returns the number of labels in the database
    #---------------------------------------------------------------------------
    def labelsCount(self):
        return db_cifar10.NB_LABELS

    #---------------------------------------------------------------------------
    # Returns the list of the names of the labels, or None if the database
    # doesn't provide that information
    #---------------------------------------------------------------------------
    def labelNamesList(self):
        return [
            'airplane',
            'automobile',
            'bird',
            'cat',
            'deer',
            'dog',
            'frog',
            'horse',
            'ship',
            'truck',
        ]

    #---------------------------------------------------------------------------
    # Returns a list of the objects in an image
    #
    # The list must contain instances of the 'Object' class
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def objectsInImage(self, index):
        (label, offset, batch) = self._imageLabel(index)
        return [Object(label, 0, 0, db_cifar10.IMAGES_WIDTH - 1, db_cifar10.IMAGES_HEIGHT - 1)]

    #---------------------------------------------------------------------------
    # Returns the name of an image file (relative to the root path where the
    # database is stored)
    #
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def imageName(self, index):
        (label, offset, batch) = self._imageLabel(index)
        if label is None:
            return None

        return '%s/%d/%04d.png' % (batch.path, label, offset)

    #---------------------------------------------------------------------------
    # Returns the size of an image
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The size as a tuple: (width, height)
    #---------------------------------------------------------------------------
    def imageSize(self, index):
        return (db_cifar10.IMAGES_WIDTH, db_cifar10.IMAGES_HEIGHT)

    #---------------------------------------------------------------------------
    # Returns the set (training or test) an image is part of
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The set, or None if not supported
    #---------------------------------------------------------------------------
    def imageSet(self, index):
        (label, offset, batch) = self._imageLabel(index)
        return batch.image_set

    #---------------------------------------------------------------------------
    # Returns the preferred image size of the database
    #
    # @return   The size as a tuple: (width, height), or None if not supported
    #---------------------------------------------------------------------------
    def preferredImageSize(self):
        return (db_cifar10.IMAGES_WIDTH, db_cifar10.IMAGES_HEIGHT)

    #---------------------------------------------------------------------------
    # Returns the preferred ROI size of the database
    #
    # @return   The size, or None if not supported
    #---------------------------------------------------------------------------
    def preferredRoiSize(self):
        return db_cifar10.IMAGES_WIDTH

    def _imageLabel(self, index):
        if index >= self.imagesCount():
            return (None, None, None)

        for batch in self.batches:
            if index < batch.start_index + batch.nb_images:
                index -= batch.start_index
                for label in range(0, db_cifar10.NB_LABELS):
                    if index < batch.nb_images_per_label[label]:
                        return (label, index, batch)

                    index -= batch.nb_images_per_label[label]

        return (None, None, None)
