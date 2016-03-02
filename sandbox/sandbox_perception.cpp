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


/** @file   sandbox_perception.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxPerception' class
*/

#include "sandbox_perception.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <iostream>


using namespace std;
using namespace Mash;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxPerception::SandboxPerception(const CommunicationChannel& channel,
                                     OutStream* pOutStream,
                                     tWardenContext* pWardenContext,
                                     bool bReadOnly)
: _channel(channel), _pWardenContext(pWardenContext), _bReadOnly(bReadOnly),
  _nbFeaturesTotal(0), _roiExtent(0), _newSequence(true)
{
    if (pOutStream)
        _outStream = *pOutStream;
}


SandboxPerception::~SandboxPerception()
{
}


/************************* HEURISTICS-RELATED METHODS *************************/

unsigned int SandboxPerception::nbHeuristics()
{
    if (_nbFeatures.empty())
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< PERCEPTION_NB_HEURISTICS" << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_PERCEPTION_NB_HEURISTICS);
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


unsigned int SandboxPerception::nbFeatures(unsigned int heuristic)
{
    if (_nbFeatures.empty() && (nbHeuristics() == 0))
        return 0;

    if (heuristic >= _nbFeatures.size())
        return 0;

    if (_nbFeatures[heuristic] == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< PERCEPTION_NB_FEATURES " << heuristic << endl;
        
        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_PERCEPTION_NB_FEATURES);
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


unsigned int SandboxPerception::nbFeaturesTotal()
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


std::string SandboxPerception::heuristicName(unsigned int heuristic)
{
    if (_nbFeatures.empty() && (nbHeuristics() == 0))
        return 0;

    if (heuristic >= _nbFeatures.size())
        return 0;

    tWardenContext* pPreviousContext = getWardenContext();
    setWardenContext(0);

    _outStream << "< PERCEPTION_HEURISTIC_NAME " << heuristic << endl;

    // Send the command to the child
    _channel.startPacket(SANDBOX_COMMAND_PERCEPTION_HEURISTIC_NAME);
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


unsigned int SandboxPerception::heuristicSeed(unsigned int heuristic)
{
    if (_seeds.empty() && (nbHeuristics() == 0))
        return 0;

    if (heuristic >= _seeds.size())
        return 0;

    if (_seeds[heuristic] == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);
 
        _outStream << "< PERCEPTION_HEURISTIC_SEED " << heuristic << endl;
        
        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_PERCEPTION_HEURISTIC_SEED);
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


bool SandboxPerception::isHeuristicUsedByModel(unsigned int heuristic)
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


/**************************** VIEWS-RELATED METHODS ***************************/

unsigned int SandboxPerception::nbViews()
{
    if (_viewSizes.empty())
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< PERCEPTION_NB_VIEWS" << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_PERCEPTION_NB_VIEWS);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the number of views
        unsigned int nbViews = 0;
        if (result && _channel.read(&nbViews))
        {
            dim_t size;
            size.width = 0;
            size.height = 0;
            
            for (unsigned int i = 0; i < nbViews; ++i)
                _viewSizes.push_back(size);
        }

        setWardenContext(pPreviousContext);
    }
    
    return _viewSizes.size();
}


bool SandboxPerception::computeSomeFeatures(unsigned int view,
                                            const coordinates_t& coordinates,
                                            unsigned int heuristic,
                                            unsigned int nbFeatures,
                                            unsigned int* indexes,
                                            scalar_t* values)
{
    // Check that the number of features is valid
    if (nbFeatures == 0)
        return false;

    // Check that the perception isn't read-only
    if (_bReadOnly)
    {
        memset(values, 0.0f, nbFeatures * sizeof(scalar_t));
        return false;
    }

    // Check that the view index is valid
    if (view >= nbViews())
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
    dim_t size = viewSize(view);
    unsigned int roi_extent = roiExtent();
    if ((coordinates.x < roi_extent) || (coordinates.x + roi_extent >= size.width) ||
        (coordinates.y < roi_extent) || (coordinates.y + roi_extent >= size.height))
        return false;

    _outStream << "< PERCEPTION_COMPUTE_SOME_FEATURES " << view << " "
               << coordinates.x << " " << coordinates.y << " "
               << heuristic << " " << nbFeatures << " ..." << endl;

    // Send the command to the child
    _channel.startPacket(SANDBOX_COMMAND_PERCEPTION_COMPUTE_SOME_FEATURES);
    _channel.add(view);
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


dim_t SandboxPerception::viewSize(unsigned int view)
{
    dim_t size;
    size.width = 0;
    size.height = 0;
    
    if (_viewSizes.empty() && (nbViews() == 0))
        return size;

    if (view >= _viewSizes.size())
        return size;

    if ((_viewSizes[view].width == 0) || (_viewSizes[view].height == 0))
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< PERCEPTION_VIEW_SIZE " << view << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_PERCEPTION_VIEW_SIZE);
        _channel.add(view);
        _channel.sendPacket();

        // Wait the response
        bool result = _channel.good();
        if (result)
            result = waitResponse();
    
        // Retrieve the size of the view
        if (result)
        {
            _channel.read(&size.width);
            _channel.read(&size.height);
        }

        if (_channel.good())
            _viewSizes[view] = size;

        setWardenContext(pPreviousContext);
    }
    
    return _viewSizes[view];
}


unsigned int SandboxPerception::roiExtent()
{
    if (_roiExtent == 0)
    {
        tWardenContext* pPreviousContext = getWardenContext();
        setWardenContext(0);

        _outStream << "< PERCEPTION_ROI_EXTENT" << endl;

        // Send the command to the child
        _channel.startPacket(SANDBOX_COMMAND_PERCEPTION_ROI_EXTENT);
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

bool SandboxPerception::waitResponse()
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
