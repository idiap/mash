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


/** @file   classifier_input_set.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'ClassifierInputSet' class
*/

#include "classifier_input_set.h"
#include "object_intersecter.h"
#include <mash/imageutils.h>
#include <algorithm>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

ClassifierInputSet::ClassifierInputSet(unsigned int maxNbSamplesInCaches,
                                       bool detection)
: _id(0), _database(maxNbSamplesInCaches), _dataset(maxNbSamplesInCaches),
  _stepper(detection), _pListener(0), _bRestrictedAccess(false), _bReadOnly(false)
{
}


ClassifierInputSet::~ClassifierInputSet()
{
}


/********************************* METHODS ************************************/

tError ClassifierInputSet::setClient(Client* pClient)
{
    // Assertions
    assert(pClient);

    return _database.setClient(pClient);
}


tError ClassifierInputSet::setParameters(const tExperimentParametersList& parameters,
                                         unsigned int imagesSeed)
{
    // Declarations
    tExperimentParametersIterator iter;
    tError ret;
    bool bUseStandardSets = true;
    float ratioTrainingSamples;
    int roiSize;
    unsigned int roiExtent;
    int step_x;
    int step_y;
    float step_z;
    ArgumentsList enabledLabels;
    bool bBackgroundImagesEnabled = false;

    _strLastError = "";

    // Retrieve the enabled labels from the parameters
    iter = parameters.find("LABELS");
    if (iter != parameters.end())
        enabledLabels = iter->second;

    // Retrieve the status of the background images from the parameters
    iter = parameters.find("BACKGROUND_IMAGES");
    if (iter != parameters.end())
        bBackgroundImagesEnabled = (iter->second.getString(0) != "OFF");

    // Retrieve the database name from the parameters
    iter = parameters.find("DATABASE_NAME");
    if (iter == parameters.end())
    {
        _strLastError = "No database selected";
        return ERROR_EXPERIMENT_PARAMETER;
    }

    ret = _database.setDatabase(iter->second.getString(0), enabledLabels, bBackgroundImagesEnabled);
    if (ret != ERROR_NONE)
        return ret;

    // Retrieves the ratio of training samples
    iter = parameters.find("TRAINING_SAMPLES");
    if (iter != parameters.end())
    {
        ratioTrainingSamples = iter->second.getFloat(0);
        bUseStandardSets = false;
    }
    else
    {
        ratioTrainingSamples = 0.5f;
    }

    if ((ratioTrainingSamples <= 0.0f) || (ratioTrainingSamples >= 1.0f))
    {
        _strLastError = "Invalid ratio of training samples";
        return ERROR_EXPERIMENT_PARAMETER;
    }

    // Retrieves the size of the region-of-interest
    iter = parameters.find("ROI_SIZE");
    if (iter != parameters.end())
    {
        roiSize = iter->second.getInt(0);
    }
    else
    {
        roiSize = _database.preferredRoiSize();
        if (roiSize == 0)
            roiSize = 127;
    }

    if (roiSize <= 1)
    {
        _strLastError = "Invalid size of the region-of-interest";
        return ERROR_EXPERIMENT_PARAMETER;
    }

    if ((roiSize % 2) == 0)
        --roiSize;

    roiExtent = (roiSize - 1) >> 1;

    // Retrieves the size of a step in the X direction
    iter = parameters.find("STEP_X");
    if (iter != parameters.end())
        step_x = iter->second.getInt(0);
    else
        step_x = (roiSize + 9) / 10;

    if (step_x <= 0)
    {
        _strLastError = "Invalid size of the step along the X direction";
        return ERROR_EXPERIMENT_PARAMETER;
    }

    // Retrieves the size of a step in the Y direction
    iter = parameters.find("STEP_Y");
    if (iter != parameters.end())
        step_y = iter->second.getInt(0);
    else
        step_y = (roiSize + 9) / 10;

    if (step_y <= 0)
    {
        _strLastError = "Invalid size of the step along the Y direction";
        return ERROR_EXPERIMENT_PARAMETER;
    }

    // Retrieves the size of a step in the Z direction
    iter = parameters.find("STEP_Z");
    if (iter != parameters.end())
        step_z = iter->second.getFloat(0);
    else
        step_z = 0.9f;

    if (step_z <= 0.0f || step_z >= 1.0f)
    {
        _strLastError = "Invalid size of the step along the Z direction";
        return ERROR_EXPERIMENT_PARAMETER;
    }

    // Setup the stepper
    _stepper.setParameters(roiExtent, step_x, step_y, step_z);

    // Setup the dataset
    _dataset.setup(&_database, roiExtent, &_stepper, bUseStandardSets,
                   ratioTrainingSamples, imagesSeed, &_computer);

    return ERROR_NONE;
}


/************************* HEURISTICS-RELATED METHODS *************************/

bool ClassifierInputSet::isHeuristicUsedByModel(unsigned int heuristic)
{
    std::vector<unsigned int>::iterator iter, iterEnd;
    
    for (iter = _heuristicsInModel.begin(), iterEnd = _heuristicsInModel.end();
         iter != iterEnd; ++iter)
    {
        if (*iter == heuristic)
            return true;
    }

    return false;
}


/*************************** IMAGES-RELATED METHODS ***************************/

bool ClassifierInputSet::computeSomeFeatures(unsigned int image,
                                             const coordinates_t& coordinates,
                                             unsigned int heuristic,
                                             unsigned int nbFeatures,
                                             unsigned int* indexes,
                                             scalar_t* values)
{
    // Assertions
    assert(_computer.initialized());

    // Check that the Input Set isn't read-only
    if (_bReadOnly)
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Check that the image index is valid
    if (image >= _dataset.nbImages())
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Check that the heuristic index is valid
    if (heuristic >= _computer.nbHeuristics())
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Retrieve the image
    Image* pImage = _dataset.getImage(image);
    if (!pImage)
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    pImage->setView(0);

    // Check that the coordinates are valid
    unsigned int roiExtent = _dataset.roiExtent();
    if ((coordinates.x < roiExtent) || (coordinates.x + roiExtent >= pImage->width()) ||
        (coordinates.y < roiExtent) || (coordinates.y + roiExtent >= pImage->height()))
        return false;

    // Compute the features
    bool success = _computer.computeSomeFeatures(_dataset.getImageIndex(image), 0,
                                                 pImage, coordinates, heuristic,
                                                 nbFeatures, indexes, values);

    // Notify the instruments
    if (_pListener && success)
    {
        _pListener->onFeaturesComputed(isDoingDetection(),
                                       _dataset.getMode() == DataSet::MODE_TRAINING,
                                       image, _dataset.getImageIndex(image),
                                       coordinates, roiExtent, heuristic,
                                       nbFeatures, indexes, values);
    }
    
    return success;
}


void ClassifierInputSet::negativesInImage(unsigned int image,
                                          tCoordinatesList* positions)
{
    // Check that we aren't in restricted mode
    if (_bRestrictedAccess)
        return;

    // Check that the image index is valid
    if (image >= _dataset.nbImages())
        return;

	// Get the roiExtent
    unsigned int roiExtent = _dataset.roiExtent();

	// Get the dimensions of the image
    dim_t image_size = imageSize(image);

	// Get the positions of the stepper
    _stepper.getPositions(image_size, positions);

    // Get the objects of the image
    tObjectsList objects;
    objectsInImage(image, &objects);

    tObjectsList::iterator iter, iterEnd;
    for (iter = objects.begin(), iterEnd = objects.end(); iter != iterEnd; ++iter)
    {
        tCoordinatesIntersecter intersecter(*iter, roiExtent);
        positions->resize(remove_if(positions->begin(), positions->end(), intersecter) - positions->begin());
    }
}
