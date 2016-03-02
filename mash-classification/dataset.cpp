/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Philip Abbet (philip.abbet@idiap.ch)
*
* This file is part of the MASH Framework.
*
* The MASH Framework is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License version 2 or
* the GNU General Public License version 3 as published by the Free
* Software Foundation, whichever suits the most your needs.
*
* The MASH Framework is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public Licenses
* along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/


/** @file   dataset.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'DataSet' class
*/

#include "dataset.h"
#include "stepper.h"
#include <mash/imageutils.h>
#include <cmath>
#include <sys/time.h>
#include <stdlib.h>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

DataSet::DataSet(unsigned int maxNbImagesInCache)
: _mode(MODE_NORMAL), _pDatabase(0), _roi_extent(0), _cache(maxNbImagesInCache)
{
}


DataSet::~DataSet()
{
    _cache.setListener(0);
}


/********************************* METHODS ************************************/

void DataSet::setup(ImageDatabase* pDatabase, unsigned int roi_extent,
                    Stepper* stepper, bool bUseStandardSets, float trainingRatio,
                    unsigned int seed, ImagesCache::IListener* pCacheListener)
{
    // Assertions
    assert(pDatabase);
    assert(roi_extent > 0);
    assert(trainingRatio > 0.0f);
    assert(trainingRatio < 1.0f);

    // Initialisations
    _pDatabase = pDatabase;
    _roi_extent = roi_extent;
    _stepper = stepper;
    _cache.setListener(pCacheListener);
    _generator.setSeed(seed);

    // Determines the number of objects used during training for each label
    vector<unsigned int> trainingCounters;
    vector<unsigned int> unassignedObjectsCounters;
    for (unsigned int i = 0; i < pDatabase->nbLabels(); ++i)
    {
        unsigned int nbObjects = pDatabase->nbObjects(i);
        unassignedObjectsCounters.push_back(nbObjects);
        trainingCounters.push_back(min(nbObjects - 1, (unsigned) max(int(nbObjects * trainingRatio), 1)));
    }

    // Populate the list used to generate the images
    unsigned int roi_size = roi_extent * 2 + 1;
    for (unsigned int image = 0; image < pDatabase->nbImages(); ++image)
    {
        dim_t image_size = pDatabase->imageSize(image);

        Stepper::tScalesList scales;
        _stepper->getScales(image_size, &scales);
        assert(scales.size() > 0);

        ImageDatabase::tObjectsList* pObjects = pDatabase->objectsOfImage(image);

        tGeneratedImagesList generatedImages;

        if (!pObjects->empty())
        {
            ImageDatabase::tObjectsIterator iter, iterEnd;
            tGeneratedImagesIterator iter2, iterEnd2;
            Stepper::tScalesIterator iter3, iterEnd3;

            // Determine in which set should go the generated images (training or test)
            vector<unsigned int> labelCounters(pDatabase->nbLabels(), 0);

            ImageDatabase::tImageSet set = (bUseStandardSets ? pDatabase->imageSet(image) : ImageDatabase::SET_NONE);

            if (set == ImageDatabase::SET_NONE)
            {
                bool training = false;
                bool test = false;

                for (iter = pObjects->begin(), iterEnd = pObjects->end(); iter != iterEnd; ++iter)
                {
                    unsigned int label = iter->label;

                    if (trainingCounters[label] == 0)
                        test = true;
                    else if (unassignedObjectsCounters[label] <= trainingCounters[label])
                        training = true;

                    ++labelCounters[label];
                    --unassignedObjectsCounters[label];
                }

                if (training)
                    set = ImageDatabase::SET_TRAINING;
                else if (test)
                    set = ImageDatabase::SET_TEST;
                else
                    set = (_generator.randomize(1.0f) < trainingRatio) ? ImageDatabase::SET_TRAINING : ImageDatabase::SET_TEST;
            }

            if (set == ImageDatabase::SET_TRAINING)
            {
                for (unsigned int label = 0; label < labelCounters.size(); ++label)
                    trainingCounters[label] -= labelCounters[label];
            }

            for (iter = pObjects->begin(), iterEnd = pObjects->end(); iter != iterEnd; ++iter)
            {
                // Compute the scale of the generated image
                unsigned int width = iter->bottom_right.x - iter->top_left.x + 1;
                unsigned int height = iter->bottom_right.y - iter->top_left.y + 1;
                unsigned int size = max(width, height);
                float scale = 1.0f;

                // Look for the scale where the size of the object is the closest
                // to the ROI size
                int smallest_diff;

                for (iter3 = scales.begin(), iterEnd3 = scales.end(); iter3 != iterEnd3; ++iter3)
                {
                    int diff = abs(int(size * (*iter3)) - int(roi_size));

                    if (iter3 == scales.begin() || diff <= smallest_diff)
                    {
                        smallest_diff = diff;
                        scale = *iter3;
                    }
                }

                iter->scale = scale;

                // Search if there is already a generated image with that scale
                if (set == ImageDatabase::SET_TRAINING || !_stepper->isDoingDetection())
                {
                    bool found = false;
                    for (iter2 = generatedImages.begin(), iterEnd2 = generatedImages.end(); iter2 != iterEnd2; ++iter2)
                    {
                        if ((iter2->scale >= scale - 1e-6f) && (iter2->scale <= scale + 1e-6f))
                        {
                            found = true;
                            break;
                        }
                    }

                    // Create the generated image if necessary
                    if (!found)
                    {
                        tGeneratedImage generatedImage;
                        generatedImage.original_image = image;
                        generatedImage.scale = scale;
                        generatedImages.push_back(generatedImage);
                    }
                }
            }


            if (_stepper->isDoingDetection() && set == ImageDatabase::SET_TEST)
            {
                for (iter3 = scales.begin(), iterEnd3 = scales.end(); iter3 != iterEnd3; ++iter3)
                {
                    tGeneratedImage generatedImage;
                    generatedImage.original_image = image;
                    generatedImage.scale = *iter3;
                    generatedImages.push_back(generatedImage);
                }
            }


            for (iter2 = generatedImages.begin(), iterEnd2 = generatedImages.end(); iter2 != iterEnd2; ++iter2)
            {
                _images.push_back(*iter2);

                if (set == ImageDatabase::SET_TRAINING)
                    _trainingImages.push_back(_images.size() - 1);
                else
                    _testImages.push_back(_images.size() - 1);
            }
        }
        else
        {
            _backgroundImages.push_back(image);
        }
    }

    // Assign the background images to one of the two sets (training or test)
    unsigned int counter = _backgroundImages.size() * trainingRatio;
    for (unsigned int i = 0; i < _backgroundImages.size(); ++i)
    {
        ImageDatabase::tImageSet set = (bUseStandardSets ? pDatabase->imageSet(_backgroundImages[i]) : ImageDatabase::SET_NONE);

        if (set == ImageDatabase::SET_NONE)
        {
            if (counter == 0)
            {
                _testImages.push_back(_images.size() + i);
            }
            else if (counter >= _backgroundImages.size() - i)
            {
                _trainingImages.push_back(_images.size() + i);
                --counter;
            }
            else if (_generator.randomize(100) > 50)
            {
                _trainingImages.push_back(_images.size() + i);
                --counter;
            }
            else
            {
                _testImages.push_back(_images.size() + i);
            }
        }
        else if (set == ImageDatabase::SET_TRAINING)
        {
            _trainingImages.push_back(_images.size() + i);
            --counter;
        }
        else
        {
            _testImages.push_back(_images.size() + i);
        }
    }
}


void DataSet::objectsOfImage(unsigned int image, tObjectsList* objects)
{
    // Assertions
    assert(image < nbImages());
    assert(objects);
    assert(_pDatabase);

    objects->clear();

    unsigned int image_index = getImageIndex(image);

    // Background image?
    if (image_index >= _images.size())
        return;

    // Retrieve the objects of the original image
    ImageDatabase::tObjectsList* pOrigObjects = _pDatabase->objectsOfImage(_images[image_index].original_image);

    // Retrieve the size of the image
    dim_t image_size = imageSize(image);
    dim_t original_size = _pDatabase->imageSize(_images[image_index].original_image);

    // Populates the result list
    float scale = _images[image_index].scale;
    unsigned int roiSize = _roi_extent * 2 + 1;
    ImageDatabase::tObjectsIterator iter, iterEnd;
    for (iter = pOrigObjects->begin(), iterEnd = pOrigObjects->end(); iter != iterEnd; ++iter)
    {
        tObject object;

        object.label = iter->label;

        object.target = (iter->scale >= scale - 1e-6f) && (iter->scale <= scale + 1e-6f);

        unsigned int top_left_x = iter->top_left.x * scale;
        unsigned int top_left_y = iter->top_left.y * scale;

        unsigned int bottom_right_x = image_size.width - 1 - (original_size.width - 1 - iter->bottom_right.x) * scale;
        unsigned int bottom_right_y = image_size.height - 1 - (original_size.height - 1 - iter->bottom_right.y) * scale;

        unsigned int width = bottom_right_x - top_left_x + 1;
        unsigned int height = bottom_right_y - top_left_y + 1;
        unsigned int size = max(width, height);

        // Ideal position
        object.roi_position.x = (top_left_x + bottom_right_x) >> 1;
        object.roi_position.y = (top_left_y + bottom_right_y) >> 1;
        object.roi_extent = (size - 1) >> 1;

        // Look for the closest coordinates (only in detection and if the object is a target)
        if (object.target && _stepper->isDoingDetection())
        {
            coordinates_t roi_position = _stepper->getClosestPosition(image_size, object.roi_position);

            // Skip objects too small
            if (object.roi_extent <= _roi_extent * _stepper->stepZ())
                object.target = false;

            // Skip objects too far from the grid
            if (abs((int) roi_position.x - (int) object.roi_position.x) >= _stepper->stepX())
                object.target = false;

            if (abs((int) roi_position.y - (int) object.roi_position.y) >= _stepper->stepY())
                object.target = false;

            object.roi_position = roi_position;
        }

        objects->push_back(object);
    }
}


Image* DataSet::getImage(unsigned int index)
{
    // Assertions
    assert(_pDatabase);
    assert(index < nbImages());

    // Declarations
    Image* pImage;

    unsigned int image_index = getImageIndex(index);

    // Look in the cache first
    pImage = _cache.getImage(image_index);
    if (pImage)
        return pImage;

    // Retrieves the original image from the database
    if (image_index < _images.size())
    {
        Image* pOriginalImage = _pDatabase->getImage(_images[image_index].original_image);
        if (!pOriginalImage)
            return 0;

        unsigned int dstWidth = pOriginalImage->width() * _images[image_index].scale;
        unsigned int dstHeight = pOriginalImage->height() * _images[image_index].scale;

        unsigned int roiSize = _roi_extent * 2 + 1;

        if (dstWidth < roiSize)
            dstWidth = roiSize;

        if (dstHeight < roiSize)
            dstHeight = roiSize;

        RGBPixel_t paddingColor = { 0 };

        pImage = ImageUtils::scale(pOriginalImage, dstWidth, dstHeight, paddingColor);
    }
    else
    {
        Image* pOriginalImage = _pDatabase->getImage(_backgroundImages[image_index - _images.size()]);
        if (!pOriginalImage)
            return 0;

        pImage = pOriginalImage->copy();
    }

    // Add it to the cache
    _cache.addImage(image_index, pImage);

    return pImage;
}


dim_t DataSet::imageSize(unsigned int image)
{
    // Assertions
    assert(_pDatabase);

    if (image >= nbImages())
    {
        dim_t size = { 0, 0 };
        return size;
    }

    unsigned int image_index = getImageIndex(image);
    unsigned int roiSize = _roi_extent * 2 + 1;

    if (image_index < _images.size())
    {
        dim_t size = _pDatabase->imageSize(_images[image_index].original_image);
        float scale = _images[image_index].scale;

        size.width *= scale;
        size.height *= scale;

        if (size.width < roiSize)
            size.width = roiSize;

        if (size.height < roiSize)
            size.height = roiSize;

        return size;
    }
    else
    {
        return _pDatabase->imageSize(_backgroundImages[image_index - _images.size()]);
    }
}


unsigned int DataSet::getImageIndex(unsigned int image)
{
    // Assertions
    assert(image < nbImages());

    if (_mode == MODE_NORMAL)
        return image;
    else if (_mode == MODE_TRAINING)
        return _trainingImages[image];
    else
        return _testImages[image];
}


void DataSet::getImageInfos(unsigned int image, std::string* strName, dim_t* size,
                            scalar_t* scale, bool* training, unsigned int* set_index)
{
    // Assertions
    assert(_pDatabase);

    if (image >= _images.size() + _backgroundImages.size())
    {
        *strName   = "";
        *scale     = 0.0f;
        *training  = false;
        *set_index = 0;
        return;
    }

    if (image < _images.size())
    {
        tGeneratedImage* pImage = &_images[image];

        *strName = _pDatabase->getImageName(pImage->original_image);
        *size = _pDatabase->imageSize(pImage->original_image);
        *scale = pImage->scale;
    }
    else
    {
        *strName = _pDatabase->getImageName(_backgroundImages[image - _images.size()]);
        *size = _pDatabase->imageSize(_backgroundImages[image - _images.size()]);
        *scale = 1.0f;
    }

    *training = false;

    for (unsigned int i = 0; i < _trainingImages.size(); ++i)
    {
        if (_trainingImages[i] == image)
        {
            *training = true;
            *set_index = i;
            break;
        }
    }

    if (!*training)
    {
        for (unsigned int i = 0; i < _testImages.size(); ++i)
        {
            if (_testImages[i] == image)
            {
                *set_index = i;
                break;
            }
        }
    }
}


bool DataSet::isImageInTestSet(unsigned int image)
{
    // Assertions
    assert(_pDatabase);

    if (image >= _images.size() + _backgroundImages.size())
        return false;

    for (unsigned int i = 0; i < _trainingImages.size(); ++i)
    {
        if (_trainingImages[i] == image)
            return false;
    }

    return true;
}
