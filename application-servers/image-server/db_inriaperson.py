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

class ImageInfos:
    def __init__(self, name, size, objects=[]):
        self.name = name
        self.size = size
        self.objects = objects


#-------------------------------------------------------------------------------
# Provides informations about the 'INRIA Person Dataset'
#-------------------------------------------------------------------------------
class db_inriaperson(Database):

    #---------------------------------------------------------------------------
    # Constructor
    #
    # @param config     Configuration of the database
    #---------------------------------------------------------------------------
    def __init__(self, config):
        super(db_inriaperson, self).__init__(config)

        self.train_images = []
        self.test_images = []
        self.labels = []

        root_path = self._setting('root_path', '')
        if root_path[-1] != '/':
            root_path += '/'

        # Open the Train annotations file
        train_annotations_file = open(os.path.join(root_path, "Train/annotations.lst"))

        for annotation in train_annotations_file:
            if annotation[-1] == '\n': # Remove trailing newline character
                annotation = annotation[0:-1]

            # Open the annotation file
            image = self.parseAnnotationFile(os.path.join(root_path, annotation))
            if image is not None:
                self.train_images.append(image)

        train_annotations_file.close()

        # Open the Test annotations file
        test_annotations_file = open(os.path.join(root_path, "Test/annotations.lst"))

        for annotation in test_annotations_file:
            if annotation[-1] == '\n': # Remove trailing newline character
                annotation = annotation[0:-1]

            # Open the annotation file
            image = self.parseAnnotationFile(os.path.join(root_path, annotation))
            if image is not None:
                self.test_images.append(image)

        test_annotations_file.close()

        # Also add all the negative images
        try:
            import Image
        except:
            raise DatabaseException("The 'inria-person' database requires the Python Imaging Library (PIL). Download it at: http://www.pythonware.com/products/pil/")

        negative_images = glob.glob(os.path.join(root_path, 'Train/neg', '*.jpg'))
        negative_images.extend(glob.glob(os.path.join(root_path, 'Train/neg', '*.png')))

        for image_name in negative_images:
            image_size = Image.open(image_name).size
            self.train_images.append(ImageInfos(image_name[len(root_path):], image_size))

        negative_images = glob.glob(os.path.join(root_path, 'Test/neg', '*.jpg'))
        negative_images.extend(glob.glob(os.path.join(root_path, 'Test/neg', '*.png')))

        for image_name in negative_images:
            image_size = Image.open(image_name).size
            self.test_images.append(ImageInfos(image_name[len(root_path):], image_size))

    #---------------------------------------------------------------------------
    # Parse an annotation file and returns the informations about the
    # corresponding image
    #---------------------------------------------------------------------------
    def parseAnnotationFile(self, filename):
        annotation_file = open(filename, 'r')

        image_name = ''
        image_size = (-1, -1)
        obj_label = -1
        objects = []

        for line in annotation_file:
            if line.startswith('Image filename'):
                index = line.find("\"", line.find(":", 14))
                image_name = line[index + 1:line.find("\"", index + 1)]
            elif line.startswith('Image size'):
                sizes = line[line.find(":", 10) + 1:].split("x")
                image_size = ( int(sizes[0]), int(sizes[1]) )
            elif line.startswith('Original label for object'):
                index = line.find("\"", line.find(":", 25))
                lbl = line[index + 1:line.find("\"", index + 1)]
                if lbl not in self.labels:
                    self.labels.append(lbl)
                obj_label = self.labels.index(lbl)
            elif line.startswith('Bounding box for object'):
                bounds = line[line.find(":", 23) + 1:].replace("(", " ").replace(")", " ").replace(",", " ").replace("-", " ").split()
                if obj_label >= 0:
                    objects.append(Object(obj_label, int(bounds[0]), int(bounds[1]), int(bounds[2]), int(bounds[3])))

        annotation_file.close()

        if (len(image_name) > 0) and (image_size[0] > 0) and (image_size[1] > 0):
            return ImageInfos(image_name, image_size, objects)
        
        return None

    #---------------------------------------------------------------------------
    # Returns the number of images in the database
    #---------------------------------------------------------------------------
    def imagesCount(self):
        return len(self.train_images) + len(self.test_images)

    #---------------------------------------------------------------------------
    # Returns the number of objects in the database
    #---------------------------------------------------------------------------
    def objectsCount(self):
        return reduce(lambda x, y: len(y.objects) + x, self.train_images, 0) + \
               reduce(lambda x, y: len(y.objects) + x, self.test_images, 0)

    #---------------------------------------------------------------------------
    # Returns the number of labels in the database
    #---------------------------------------------------------------------------
    def labelsCount(self):
        return len(self.labels)

    #---------------------------------------------------------------------------
    # Returns the list of the names of the labels, or None if the database
    # doesn't provide that information
    #---------------------------------------------------------------------------
    def labelNamesList(self):
        return self.labels

    #---------------------------------------------------------------------------
    # Returns a list of the objects in an image
    #
    # The list must contain instances of the 'Object' class
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def objectsInImage(self, index):
        if index < len(self.train_images):
            return self.train_images[index].objects
        else:
            return self.test_images[index - len(self.train_images)].objects

    #---------------------------------------------------------------------------
    # Returns the name of an image file (relative to the root path where the
    # database is stored)
    #
    # @param index  Index of the image (0 .. imagesCount() - 1)
    #---------------------------------------------------------------------------
    def imageName(self, index):
        if index < len(self.train_images):
            return self.train_images[index].name
        else:
            return self.test_images[index - len(self.train_images)].name

    #---------------------------------------------------------------------------
    # Returns the size of an image
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The size as a tuple: (width, height)
    #---------------------------------------------------------------------------
    def imageSize(self, index):
        if index < len(self.train_images):
            return self.train_images[index].size
        else:
            return self.test_images[index - len(self.train_images)].size

    #---------------------------------------------------------------------------
    # Returns the set (training or test) an image is part of
    #
    # @param    index   Index of the image (0 .. imagesCount() - 1)
    # @return           The set, or None if not supported
    #---------------------------------------------------------------------------
    def imageSet(self, index):
        if index < len(self.train_images):
            return Database.TRAINING_SET
        else:
            return Database.TEST_SET

    #---------------------------------------------------------------------------
    # Returns the preferred ROI size of the database
    #
    # @return   The size, or None if not supported
    #---------------------------------------------------------------------------
    def preferredRoiSize(self):
        return 128
