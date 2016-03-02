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


from database import Database, Object, DatabaseException
import glob
import os
import pickle


#-------------------------------------------------------------------------------
# Provides informations about the 'caltech-256' database
#-------------------------------------------------------------------------------
class db_caltech256(Database):
    
    NB_LABELS = 256
    PREFERRED_SIZE = 127
    
    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param config     Configuration of the database
    #---------------------------------------------------------------------------
    def __init__(self, config):
        super(db_caltech256, self).__init__(config)

        # Note: we have only one object per image

        self.labels = []
        self.images = []

        root_path = self._setting('root_path', '')
        original_folder = self._setting('original_images', '')
        
        if original_folder.endswith('/'):
            original_folder = original_folder[0:-1]

        try:
            if os.path.exists('db_caltech256.data'):
                pkl_file = open('db_caltech256.data', 'rb')
                self.labels = pickle.load(pkl_file)
                self.images = pickle.load(pkl_file)
                pkl_file.close()
        except KeyboardInterrupt:
            raise
        except:
            self.labels = []
            self.images = []
            
        if len(self.labels) == 0:
            try:
                import Image
            except:
                raise DatabaseException("The 'caltech-256' database requires the Python Imaging Library (PIL). Download it at: http://www.pythonware.com/products/pil/")

            for i in range(0, db_caltech256.NB_LABELS):
                folder = glob.glob(os.path.join(root_path, original_folder, '%03d.*' % (i + 1)))[0]

                files = glob.glob(os.path.join(folder, '%03d_*.jpg' % (i + 1)))

                label_name = os.path.basename(folder)[4:]
                if label_name.endswith('-101'):
                    label_name = label_name[:-4]

                self.labels.append((label_name, len(files)))

                for filename in files:
                    size = Image.open(filename).size
                    self.images.append((filename[len(os.path.join(root_path, original_folder))+1:].replace('.jpg', '.png'), size, Object(len(self.labels) - 1, 0, 0, size[0] - 1, size[1] - 1)))

            pkl_file = open('db_caltech256.data', 'wb')
            pickle.dump(self.labels, pkl_file, -1)
            pickle.dump(self.images, pkl_file, -1)
            pkl_file.close()
    
    #---------------------------------------------------------------------------
    # Returns the number of images in the database
    #---------------------------------------------------------------------------
    def imagesCount(self):
        return len(self.images)

    #---------------------------------------------------------------------------
    # Returns the number of objects in the database
    #---------------------------------------------------------------------------
    def objectsCount(self):
        return len(self.images)

    #---------------------------------------------------------------------------
    # Returns the number of labels in the database
    #---------------------------------------------------------------------------
    def labelsCount(self):
        return db_caltech256.NB_LABELS

    #---------------------------------------------------------------------------
    # Returns the list of the names of the labels, or None if the database
    # doesn't provide that information
    #---------------------------------------------------------------------------
    def labelNamesList(self):
        return map(lambda x: x[0], self.labels)

    #---------------------------------------------------------------------------
    # Returns a list of the objects in an image
    #
    # The list must contain instances of the 'Object' class
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def objectsInImage(self, index):
        if index < len(self.images):
            if self.use_preprocessed_images:
                return [Object(self.images[index][2].label, 0, 0, db_caltech256.PREFERRED_SIZE - 1, db_caltech256.PREFERRED_SIZE - 1)]
            else:
                return [self.images[index][2]]
        
        return []

    #---------------------------------------------------------------------------
    # Returns the name of an image file (relative to the root path where the
    # database is stored)
    #
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def imageName(self, index):
        if index < len(self.images):
            if self.use_preprocessed_images:
                return self.images[index][0]
            else:
                return self.images[index][0].replace('.png', '.jpg')
        
        return None

    #---------------------------------------------------------------------------
    # Returns the size of an image
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The size as a tuple: (width, height)
    #---------------------------------------------------------------------------
    def imageSize(self, index):
        if index < len(self.images):
            if self.use_preprocessed_images:
                return (db_caltech256.PREFERRED_SIZE, db_caltech256.PREFERRED_SIZE)
            else:
                return self.images[index][1]
        
        return None

    #---------------------------------------------------------------------------
    # Returns the preferred image size of the database
    #
    # @return   The size as a tuple: (width, height), or None if not supported
    #---------------------------------------------------------------------------
    def preferredImageSize(self):
        return (db_caltech256.PREFERRED_SIZE, db_caltech256.PREFERRED_SIZE)

    #---------------------------------------------------------------------------
    # Returns the preferred ROI size of the database
    #
    # @return   The size, or None if not supported
    #---------------------------------------------------------------------------
    def preferredRoiSize(self):
        return db_caltech256.PREFERRED_SIZE
