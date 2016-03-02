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


/** @file   sandbox_input_set.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxInputSet' class
*/

#include "sandbox_input_set.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <iostream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxInputSet::SandboxInputSet(const CommunicationChannel& channel,
                                 Mash::OutStream* pOutStream,
                                 tWardenContext* pWardenContext,
                                 bool bReadOnly)
: _channel(channel), _pWardenContext(pWardenContext), _bReadOnly(bReadOnly),
  _id(0), _bDoingDetection(false), _nbFeaturesTotal(0), _nbLabels(0),
  _roiExtent(0)
{
    if (pOutStream)
        _outStream = *pOutStream;
}


SandboxInputSet::~SandboxInputSet()
{
}


/************************* HEURISTICS-RELATED METHODS *************************/

unsigned int SandboxInputSet::nbHeuristics()
{
    if (_nbFeatures.empty())
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< INPUT_SET_NB_HEURISTICS" << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_NB_HEURISTICS);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the number of heuristics
        unsigned int nbHeuristics = 0;
        if (result && _channel.read(&nbHeuristics))
        {
            for (unsigned int i = 0; i < nbHeuristics; ++i)
            {
                _nbFeatures.push_back(0);
                _seeds.push_back(0);
            }
        }

        setWardenContext(pPreviousContext);
    }
    

    return _nbFeatures.size();
}


unsigned int SandboxInputSet::nbFeatures(unsigned int heuristic)
{
    if (_nbFeatures.empty() && (nbHeuristics() == 0))
        return 0;

    if (heuristic >= _nbFeatures.size())
        return 0;

    if (_nbFeatures[heuristic] == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);
 
        _outStream << "< INPUT_SET_NB_FEATURES " << heuristic << endl;
        
        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_NB_FEATURES);
        _channel.add(heuristic);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the number of features
        unsigned int nbFeatures = 0;
        if (result && _channel.read(&nbFeatures))
            _nbFeatures[heuristic] = nbFeatures;

        setWardenContext(pPreviousContext);
    }
    
    return _nbFeatures[heuristic];
}


unsigned int SandboxInputSet::nbFeaturesTotal()
{
    if (_nbFeatures.empty() && (nbHeuristics() == 0))
        return 0;

    if (_nbFeaturesTotal == 0)
    {
        for (unsigned int i = 0; i < nbHeuristics(); ++i)
            _nbFeaturesTotal += nbFeatures(i);
    }
    
    return _nbFeaturesTotal;
}


std::string SandboxInputSet::heuristicName(unsigned int heuristic)
{
    if (_nbFeatures.empty() && (nbHeuristics() == 0))
        return 0;

    if (heuristic >= _nbFeatures.size())
        return 0;

    tWardenContext* pPreviousContext = getWardenContext();
    setWardenContext(0);

    _outStream << "< INPUT_SET_HEURISTIC_NAME " << heuristic << endl;

    // Send the command to the child
    _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_HEURISTIC_NAME);
    _channel.add(heuristic);
    _channel.sendPacket();

    // Wait the response
    bool result = _channel.good();
    if (result)
        result = waitResponse();
    
    setWardenContext(pPreviousContext);

    // Retrieve the name
    string strName;
    if (result)
        _channel.read(&strName);
    
    return strName;
}


unsigned int SandboxInputSet::heuristicSeed(unsigned int heuristic)
{
    if (_seeds.empty() && (nbHeuristics() == 0))
        return 0;

    if (heuristic >= _seeds.size())
        return 0;

    if (_seeds[heuristic] == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);
 
        _outStream << "< INPUT_SET_HEURISTIC_SEED " << heuristic << endl;
        
        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_HEURISTIC_SEED);
        _channel.add(heuristic);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the seed
        unsigned int seed = 0;
        if (result && _channel.read(&seed))
            _seeds[heuristic] = seed;

        setWardenContext(pPreviousContext);
    }
    
    return _seeds[heuristic];
}


bool SandboxInputSet::isHeuristicUsedByModel(unsigned int heuristic)
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

unsigned int SandboxInputSet::nbImages()
{
    if (_imageSizes.empty())
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< INPUT_SET_NB_IMAGES" << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_NB_IMAGES);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the number of images
        unsigned int nbImages = 0;
        if (result && _channel.read(&nbImages))
        {
            dim_t size;
            size.width = 0;
            size.height = 0;
            
            for (unsigned int i = 0; i < nbImages; ++i)
            {
                _imageSizes.push_back(size);
                _imageInTestSet.push_back(-1);
            }
        }

        setWardenContext(pPreviousContext);
    }
    
    return _imageSizes.size();
}


unsigned int SandboxInputSet::nbLabels()
{
    if (_nbLabels == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< INPUT_SET_NB_LABELS" << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_NB_LABELS);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the number of images
        if (result)
            _channel.read(&_nbLabels);

        setWardenContext(pPreviousContext);
    }
    
    return _nbLabels;
}


bool SandboxInputSet::computeSomeFeatures(unsigned int image,
                                          const coordinates_t& coordinates,
                                          unsigned int heuristic,
                                          unsigned int nbFeatures,
                                          unsigned int* indexes,
                                          scalar_t* values)
{
    // Check that the number of features is valid
    if (nbFeatures == 0)
        return false;

    // Check that the Input Set isn't read-only
    if (_bReadOnly)
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Check that the image index is valid
    if (image >= nbImages())
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Check that the heuristic index is valid
    if (heuristic >= nbHeuristics())
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    tWardenContext* pPreviousContext = getWardenContext();
    setWardenContext(0);

    // Check that the coordinates are valid
    dim_t size = imageSize(image);
    unsigned int roi_extent = roiExtent();
    if ((coordinates.x < roi_extent) || (coordinates.x + roi_extent >= size.width) ||
        (coordinates.y < roi_extent) || (coordinates.y + roi_extent >= size.height))
        return false;

    _outStream << "< INPUT_SET_COMPUTE_SOME_FEATURES " << " " << image << " "
               << coordinates.x << " " << coordinates.y << " "
               << heuristic << " " << nbFeatures << " ..." << endl;

    // Send the command to the child
    _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_COMPUTE_SOME_FEATURES);
    _channel.add(image);
    _channel.add(coordinates.x);
    _channel.add(coordinates.y);
    _channel.add(heuristic);
    _channel.add(nbFeatures);
    _channel.add((char*) indexes, nbFeatures * sizeof(unsigned int));
    _channel.sendPacket();

    // Read the response
    bool result = _channel.good();
    if (result)
        result = waitResponse();

    if (result)
        _channel.read((char*) values, nbFeatures * sizeof(scalar_t));

    setWardenContext(pPreviousContext);
        
    return result && _channel.good();
}


void SandboxInputSet::objectsInImage(unsigned int image, tObjectsList* objects)
{
    objects->clear();
    
    if (_imageSizes.empty() && (nbImages() == 0))
        return;

    if (image >= _imageSizes.size())
        return;

    tWardenContext* pPreviousContext = getWardenContext();
    setWardenContext(0);

    _outStream << "< INPUT_SET_OBJECTS_IN_IMAGE " << image << endl;

    // Send the command to the child
    _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_OBJECTS_IN_IMAGE);
    _channel.add(image);
    _channel.sendPacket();

    // Wait the response
    bool result = _channel.good();
    if (result)
        result = waitResponse();

    // Retrieve the objects of the image
    if (result)
    {
        unsigned int nbObjects = 0;
        _channel.read(&nbObjects);

        setWardenContext(pPreviousContext);
        for (unsigned int i = 0; _channel.good() && (i < nbObjects); ++i)
        {
            tObject object;
            if (_channel.read((char*) &object, sizeof(tObject)))
                objects->push_back(object);
        }
        setWardenContext(0);
    }

    setWardenContext(pPreviousContext);
}


void SandboxInputSet::negativesInImage(unsigned int image, tCoordinatesList* positions)
{
    positions->clear();
    
    if (_imageSizes.empty() && (nbImages() == 0))
        return;

    if (image >= _imageSizes.size())
        return;

    tWardenContext* pPreviousContext = getWardenContext();
    setWardenContext(0);

    _outStream << "< INPUT_SET_NEGATIVES_IN_IMAGE " << image << endl;

    // Send the command to the child
    _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_NEGATIVES_IN_IMAGE);
    _channel.add(image);
    _channel.sendPacket();

    // Wait the response
    bool result = _channel.good();
    if (result)
        result = waitResponse();

    // Retrieve the positions of the negatives
    if (result)
    {
        unsigned int nbPositions = 0;
        _channel.read(&nbPositions);

        setWardenContext(pPreviousContext);
        for (unsigned int i = 0; _channel.good() && (i < nbPositions); ++i)
        {
            coordinates_t coords;
            if (_channel.read((char*) &coords, sizeof(coordinates_t)))
                positions->push_back(coords);
        }
        setWardenContext(0);
    }
    
    setWardenContext(pPreviousContext);
}


dim_t SandboxInputSet::imageSize(unsigned int image)
{
    dim_t size;
    size.width = 0;
    size.height = 0;
    
    if (_imageSizes.empty() && (nbImages() == 0))
        return size;

    if (image >= _imageSizes.size())
        return size;

    if ((_imageSizes[image].width == 0) || (_imageSizes[image].height == 0))
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< INPUT_SET_IMAGE_SIZE " << image << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_IMAGE_SIZE);
        _channel.add(image);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the size of the image
        if (result)
        {
            _channel.read(&size.width);
            _channel.read(&size.height);
        }

        if (_channel.good())
            _imageSizes[image] = size;

        setWardenContext(pPreviousContext);
    }
    
    return _imageSizes[image];
}


bool SandboxInputSet::isImageInTestSet(unsigned int image)
{
    if (_imageInTestSet.empty() && (nbImages() == 0))
        return false;

    if (image >= _imageInTestSet.size())
        return false;

    if (_imageInTestSet[image] == -1)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< INPUT_SET_IMAGE_IN_TEST_SET " << image << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_IMAGE_IN_TEST_SET);
        _channel.add(image);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the size of the image
        bool inTestSet = false;
        if (result)
            _channel.read(&inTestSet);

        if (_channel.good())
            _imageInTestSet[image] = (inTestSet ? 1 : 0);

        setWardenContext(pPreviousContext);
    }
    
    return (_imageInTestSet[image] == 1);
}


unsigned int SandboxInputSet::roiExtent()
{
    if (_roiExtent == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< INPUT_SET_ROI_EXTENT" << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_INPUT_SET_ROI_EXTENT);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the size of the image
        if (result)
            _channel.read(&_roiExtent);

        setWardenContext(pPreviousContext);
    }
    
    return _roiExtent;
}


/********************** COMMUNICATION_RELATED METHODS *************************/

bool SandboxInputSet::waitResponse()
{
    // Declarations
    tSandboxMessage message;
    
    while (_channel.good())
    {
        if (_channel.receivePacket(&message) != ERROR_NONE)
            break;
        
        if (message == SANDBOX_MESSAGE_RESPONSE)
            return true;

        if (message == SANDBOX_COMMAND_TERMINATE)
        {
            _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
            _channel.sendPacket();
            _exit(0);
        }

        if (message == SANDBOX_MESSAGE_ERROR)
        {
            tError error;
            _channel.read(&error);
            _outStream << "> ERROR " << getErrorDescription(error) << endl;
            break;
        }

        if (message != SANDBOX_MESSAGE_KEEP_ALIVE)
        {
            _outStream << getErrorDescription(ERROR_CHANNEL_UNEXPECTED_RESPONSE) << ": " << message << endl;
            break;
        }
    }
    
    // Error handling
    return false;
}
