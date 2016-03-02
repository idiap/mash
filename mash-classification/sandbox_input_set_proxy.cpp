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


/** @file   sandbox_input_set_proxy.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxInputSetProxy' class
*/

#include "sandbox_input_set_proxy.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-classification/classifier_input_set.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <sstream>


using namespace std;
using namespace Mash;
using namespace Mash::SandboxControllerDeclarations;


/****************************** STATIC ATTRIBUTES *****************************/

SandboxInputSetProxy::tCommandHandlersList SandboxInputSetProxy::handlers;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxInputSetProxy::SandboxInputSetProxy(IClassifierInputSet* pInputSet, const CommunicationChannel& channel)
: _channel(channel), _pInputSet(pInputSet)
{
    assert(pInputSet);
    
    if (handlers.empty())
    {
        handlers[SANDBOX_COMMAND_INPUT_SET_NB_HEURISTICS]            = &SandboxInputSetProxy::handleInputSetNbHeuristicsCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_NB_FEATURES]              = &SandboxInputSetProxy::handleInputSetNbFeaturesCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_HEURISTIC_NAME]           = &SandboxInputSetProxy::handleInputSetHeuristicNameCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_HEURISTIC_SEED]           = &SandboxInputSetProxy::handleInputSetHeuristicSeedCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_NB_IMAGES]                = &SandboxInputSetProxy::handleInputSetNbImagesCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_NB_LABELS]                = &SandboxInputSetProxy::handleInputSetNbLabelsCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_COMPUTE_SOME_FEATURES]    = &SandboxInputSetProxy::handleInputSetComputeSomeFeaturesCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_OBJECTS_IN_IMAGE]         = &SandboxInputSetProxy::handleInputSetObjectsInImageCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_NEGATIVES_IN_IMAGE]       = &SandboxInputSetProxy::handleInputSetNegativesInImageCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_IMAGE_SIZE]               = &SandboxInputSetProxy::handleInputSetImageSizeCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_IMAGE_IN_TEST_SET]        = &SandboxInputSetProxy::handleInputSetImageInTestSetCommand;
        handlers[SANDBOX_COMMAND_INPUT_SET_ROI_EXTENT]               = &SandboxInputSetProxy::handleInputSetRoiExtentCommand;
    }
}


SandboxInputSetProxy::~SandboxInputSetProxy()
{
}


/**************************** COMMAND HANDLERS ********************************/

tCommandProcessingResult SandboxInputSetProxy::processResponse(tSandboxMessage message)
{
    // Assertions
    assert(_pInputSet);
    assert(!handlers.empty());

    tCommandHandlersIterator iter = handlers.find(message);
    if (iter != handlers.end())
    {
        tCommandHandler handler = iter->second;
        return (this->*handler)();
    }

    return COMMAND_UNKNOWN;
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetNbHeuristicsCommand()
{
    // Assertions
    assert(_pInputSet);
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pInputSet->nbHeuristics());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetNbFeaturesCommand()
{
    // Assertions
    assert(_pInputSet);
    
    // Retrieve the index of the heuristic
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
        return SOURCE_PLUGIN_CRASHED;

    // Retrieve the number of features
    unsigned int nbFeatures = _pInputSet->nbFeatures(heuristic);

    if (dynamic_cast<ClassifierInputSet*>(_pInputSet))
    {
        tError error = dynamic_cast<ClassifierInputSet*>(_pInputSet)->getLastHeuristicsError();
        
        if (error == ERROR_HEURISTIC_TIMEOUT)
            return DEST_PLUGIN_TIMEOUT;
        else if (error != ERROR_NONE)
            return DEST_PLUGIN_CRASHED;
    }
    else if (nbFeatures == 0)
    {
        return DEST_PLUGIN_CRASHED;
    }

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(nbFeatures);
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetHeuristicNameCommand()
{
    // Assertions
    assert(_pInputSet);

    // Retrieve the index of the heuristic
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
        return SOURCE_PLUGIN_CRASHED;

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pInputSet->heuristicName(heuristic));
    _channel.sendPacket();

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetHeuristicSeedCommand()
{
    // Assertions
    assert(_pInputSet);

    // Retrieve the index of the heuristic
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
        return SOURCE_PLUGIN_CRASHED;

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pInputSet->heuristicSeed(heuristic));
    _channel.sendPacket();

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetNbImagesCommand()
{
    // Assertions
    assert(_pInputSet);
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pInputSet->nbImages());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetNbLabelsCommand()
{
    // Assertions
    assert(_pInputSet);
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pInputSet->nbLabels());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetComputeSomeFeaturesCommand()
{
    // Assertions
    assert(_pInputSet);

    // Declarations
    unsigned int    image;
    coordinates_t   coordinates;
    unsigned int    heuristic;
    unsigned int    nbFeatures;
    unsigned int*   indexes = 0;
    scalar_t*       values = 0;

    // Retrieve all the parameters
    _channel.read(&image);
    _channel.read(&coordinates.x);
    _channel.read(&coordinates.y);
    _channel.read(&heuristic);
    _channel.read(&nbFeatures);

    if (!_channel.good())
        return SOURCE_PLUGIN_CRASHED;

    if (nbFeatures == 0)
        return INVALID_ARGUMENTS;

    indexes = new unsigned int[nbFeatures];
    values = new scalar_t[nbFeatures];

    _channel.read((char*) indexes, nbFeatures * sizeof(unsigned int));

    if (!_channel.good())
    {
        delete[] indexes;
        delete[] values;
        return SOURCE_PLUGIN_CRASHED;
    }

    // Compute the features
    bool success = _pInputSet->computeSomeFeatures(image, coordinates, heuristic, nbFeatures, indexes, values);
    if (!success)
    {
        delete[] indexes;
        delete[] values;

        if (dynamic_cast<ClassifierInputSet*>(_pInputSet))
        {
            tError error = dynamic_cast<ClassifierInputSet*>(_pInputSet)->getLastHeuristicsError();

            if (error == ERROR_HEURISTIC_TIMEOUT)
                return DEST_PLUGIN_TIMEOUT;
            else if (error == ERROR_NONE)
                return INVALID_ARGUMENTS;
        }

        return DEST_PLUGIN_CRASHED;
    }

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add((char*) values, nbFeatures * sizeof(scalar_t));
    _channel.sendPacket();

    delete[] indexes;
    delete[] values;

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetObjectsInImageCommand()
{
    // Assertions
    assert(_pInputSet);

    // Retrieve the index of the image
    unsigned int image;
    if (!_channel.read(&image))
        return SOURCE_PLUGIN_CRASHED;

    // Retrieve the objects in the image
    tObjectsList objects;
    _pInputSet->objectsInImage(image, &objects);

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);

    unsigned int nbObjects = objects.size();
    _channel.add(nbObjects);

    for (unsigned int i = 0; _channel.good() && (i < nbObjects); ++i)
        _channel.add((char*) &objects[i], sizeof(tObject));

    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetNegativesInImageCommand()
{
    // Assertions
    assert(_pInputSet);

    // Retrieve the index of the image
    unsigned int image;
    if (!_channel.read(&image))
        return SOURCE_PLUGIN_CRASHED;

    // Retrieve the objects in the image
    tCoordinatesList positions;
    _pInputSet->negativesInImage(image, &positions);

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);

    unsigned int nbPositions = positions.size();
    _channel.add(nbPositions);

    for (unsigned int i = 0; _channel.good() && (i < nbPositions); ++i)
        _channel.add((char*) &positions[i], sizeof(coordinates_t));

    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetImageSizeCommand()
{
    // Assertions
    assert(_pInputSet);

    // Retrieve the index of the image
    unsigned int image;
    if (!_channel.read(&image))
        return SOURCE_PLUGIN_CRASHED;

    // Retrieve the size of the image
    dim_t size = _pInputSet->imageSize(image);

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(size.width);
    _channel.add(size.height);
    _channel.sendPacket();

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetImageInTestSetCommand()
{
    // Assertions
    assert(_pInputSet);

    // Retrieve the index of the image
    unsigned int image;
    if (!_channel.read(&image))
        return SOURCE_PLUGIN_CRASHED;

    // Retrieve the information about the image
    bool inTestSet = _pInputSet->isImageInTestSet(image);

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(inTestSet);
    _channel.sendPacket();

    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}


tCommandProcessingResult SandboxInputSetProxy::handleInputSetRoiExtentCommand()
{
    // Assertions
    assert(_pInputSet);
    
    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(_pInputSet->roiExtent());
    _channel.sendPacket();
    
    return (_channel.good() ? COMMAND_PROCESSED : SOURCE_PLUGIN_CRASHED);
}
