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


/** @file   sandboxed_heuristics_set.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxedHeuristicsSet' class
*/

#include "sandboxed_heuristics_set.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-sandboxing/declarations.h>
#include <mash-utils/stringutils.h>
#include <mash-utils/errors.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <sstream>


using namespace std;
using namespace Mash;
using namespace Mash::SandboxControllerDeclarations;
using namespace Mash::SandboxTimeBudgetDeclarations;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxedHeuristicsSet::SandboxedHeuristicsSet()
: _currentHeuristic(-1), _last_sent_sequence(-1), _last_sent_image_index(-1),
  _lastError(ERROR_NONE)
{
}


SandboxedHeuristicsSet::~SandboxedHeuristicsSet()
{
    _outStream.deleteFile();
}


/***************************** SANDBOX MANAGEMENT *****************************/

bool SandboxedHeuristicsSet::createSandbox(const tSandboxConfiguration& configuration)
{
    _outStream.setVerbosityLevel(3);
    _outStream.open("HeuristicsSandboxController",
                    configuration.strLogDir + "HeuristicsSandboxController_$TIMESTAMP.log",
                    200 * 1024);

    _sandbox.setOutputStream(_outStream);
    _sandbox.addLogFileInfos("HeuristicsSandbox");

    return _sandbox.createSandbox(PLUGIN_HEURISTIC, configuration, this);
}


/**************************** INSTRUMENTS MANAGEMENT **************************/

bool SandboxedHeuristicsSet::setHeuristicsFolder(const std::string& strPath)
{
    return _sandbox.setPluginsFolder(strPath);
}


int SandboxedHeuristicsSet::loadHeuristicPlugin(const std::string& strName)
{
    // Save the context (in case of crash)
    std::ostringstream str;
    str << "Method: loading" << endl;
	_strContext = str.str();

    return _sandbox.loadPlugin(strName);
}


bool SandboxedHeuristicsSet::createHeuristics()
{
    // Save the context (in case of crash)
    std::ostringstream str;
    str << "Method: constructor" << endl;
	_strContext = str.str();
        
    return _sandbox.createPlugins();
}


unsigned int SandboxedHeuristicsSet::nbHeuristics() const
{
    return _sandbox.nbPlugins();
}


int SandboxedHeuristicsSet::heuristicIndex(const std::string& strName) const
{
    return _sandbox.getPluginIndex(strName);
}


std::string SandboxedHeuristicsSet::heuristicName(int index) const
{
    return _sandbox.getPluginName(index);
}


/*********************************** METHODS **********************************/

bool SandboxedHeuristicsSet::setSeed(unsigned int heuristic, unsigned int seed)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< SET_SEED " << heuristic << " " << seed << endl;

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();
    
    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_SET_SEED);
    pChannel->add(heuristic);
    pChannel->add(seed);
    pChannel->sendPacket();

    if (!pChannel->good())
        return false;

    // Read the response
    return _sandbox.waitResponse(TIMEOUT_SANDBOX);
}


bool SandboxedHeuristicsSet::init(unsigned int heuristic, unsigned int nb_views,
                                  unsigned int roi_extent)
{
    if (getLastError() != ERROR_NONE)
        return false;

    while (heuristic >= _contexts.size())
    {
        tContext context;
        memset(&context, 0, sizeof(context));
        _contexts.push_back(context);
    }

    _outStream << "< INIT " << heuristic << " " << nb_views << " " << roi_extent << endl;

    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context     = _contexts[heuristic];
    context.nb_views     = nb_views;
    context.roi_extent   = roi_extent;
    _contexts[heuristic] = context;

    std::ostringstream str;
	
	str << "Method: init" << endl
        << "Parameters:" << endl
        << "    - Number of views: " << context.nb_views << endl
        << "    - ROI extent:      " << context.roi_extent << " pixels" << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_INIT);
    pChannel->add(heuristic);
    pChannel->add(nb_views);
    pChannel->add(roi_extent);
    pChannel->sendPacket();

    if (!pChannel->good())
        return false;

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;

    return result;
}


unsigned int SandboxedHeuristicsSet::dim(unsigned int heuristic)
{
    if (getLastError() != ERROR_NONE)
        return 0;

    _outStream << "< DIM " << heuristic << endl;

    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context = _contexts[heuristic];

    std::ostringstream str;
	
	str << "Method: dim" << endl
        << "Parameters:" << endl
        << "    - Number of views: " << context.nb_views << endl
        << "    - ROI extent:      " << context.roi_extent << " pixels" << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_DIM);
    pChannel->add(heuristic);
    pChannel->sendPacket();

    // Read the response
    unsigned int dimension = 0;

    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    if (result)
    {
        pChannel->read(&dimension);
    
        if (pChannel->good())
            _outStream << "> RESPONSE: " << dimension << endl;
        else
            _outStream << getErrorDescription(pChannel->getLastError()) << endl;
    }

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;
    
    return (_lastError == ERROR_NONE ? dimension : 0);
}


bool SandboxedHeuristicsSet::prepareForSequence(unsigned int heuristic)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< PREPARE_FOR_SEQUENCE " << heuristic << endl;

    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context = _contexts[heuristic];
    ++context.sequence;
    _contexts[heuristic] = context;

    std::ostringstream str;
	
	str << "Method: prepareForSequence" << endl
        << "Parameters:" << endl
        << "    - Number of views: " << context.nb_views << endl
        << "    - ROI extent:      " << context.roi_extent << " pixels" << endl
        << "    - Sequence:        #" << context.sequence << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_SEQUENCE);
    pChannel->add(heuristic);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;

    return result;
}


bool SandboxedHeuristicsSet::finishForSequence(unsigned int heuristic)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< FINISH_FOR_SEQUENCE " << heuristic << endl;

    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context = _contexts[heuristic];

    std::ostringstream str;
	
	str << "Method: finishForSequence" << endl
        << "Parameters:" << endl
        << "    - Number of views: " << context.nb_views << endl
        << "    - ROI extent:      " << context.roi_extent << " pixels" << endl
        << "    - Sequence:        #" << context.sequence << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_SEQUENCE);
    pChannel->add(heuristic);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;

    return result;
}


bool SandboxedHeuristicsSet::prepareForImage(unsigned int heuristic,
                                             unsigned int sequence,
                                             unsigned int image_index,
                                             const Image* image)
{
    // Assertions
    assert(image);
    assert(image->width());
    assert(image->height());

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< PREPARE_FOR_IMAGE " << heuristic << " " << image->width()
               << " " << image->height() << " " << image->view() << " "
               << image->pixelFormats() << " ..."<< endl;

    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context        = _contexts[heuristic];
    context.image_width     = image->width();
    context.image_height    = image->height();
    _contexts[heuristic]    = context;

    std::ostringstream str;
	
	str << "Method: prepareForImage" << endl
        << "Parameters:" << endl
        << "    - Number of views: " << context.nb_views << endl
        << "    - ROI extent:      " << context.roi_extent << " pixels" << endl
        << "    - Sequence:        #" << context.sequence << endl
        << "    - Image size:      " << context.image_width << "x" << context.image_height << " pixels" << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_IMAGE);
    pChannel->add(heuristic);
    
    if ((_last_sent_sequence != sequence) || (_last_sent_image_index != image_index))
    {
        pChannel->add(image->width());
        pChannel->add(image->height());
        pChannel->add(image->view());
        pChannel->add(image->pixelFormats());

        if (image->hasPixelFormat(Image::PIXELFORMAT_RGB))
            pChannel->add((char*) image->rgbBuffer(), image->width() * image->height() * sizeof(RGBPixel_t));

        if (image->hasPixelFormat(Image::PIXELFORMAT_GRAY))
            pChannel->add((char*) image->grayBuffer(), image->width() * image->height() * sizeof(byte_t));

        _last_sent_sequence    = sequence;
        _last_sent_image_index = image_index;
    }
    else
    {
        pChannel->add(0);
    }

    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;

    return result;
}


bool SandboxedHeuristicsSet::finishForImage(unsigned int heuristic)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< FINISH_FOR_IMAGE " << heuristic << endl;

    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context = _contexts[heuristic];

    std::ostringstream str;
	
	str << "Method: finishForImage" << endl
        << "Parameters:" << endl
        << "    - Number of views: " << context.nb_views << endl
        << "    - ROI extent:      " << context.roi_extent << " pixels" << endl
        << "    - Sequence:        #" << context.sequence << endl
        << "    - Image size:      " << context.image_width << "x" << context.image_height << " pixels" << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_IMAGE);
    pChannel->add(heuristic);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;

    return result;
}


bool SandboxedHeuristicsSet::prepareForCoordinates(unsigned int heuristic,
                                                   const coordinates_t& coordinates)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< PREPARE_FOR_COORDINATES " << heuristic << " "
               << coordinates.x << " " << coordinates.x << endl;
    
    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context        = _contexts[heuristic];
    context.coordinates     = coordinates;
    _contexts[heuristic]    = context;

    std::ostringstream str;
	
	str << "Method: prepareForCoordinates" << endl
        << "Parameters:" << endl
        << "    - Number of views: " << context.nb_views << endl
        << "    - ROI extent:      " << context.roi_extent << " pixels" << endl
        << "    - Sequence:        #" << context.sequence << endl
        << "    - Image size:      " << context.image_width << "x" << context.image_height << " pixels" << endl
        << "    - ROI position:    (" << context.coordinates.x << ", " << context.coordinates.y << ")" << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_COORDINATES);
    pChannel->add(heuristic);
    pChannel->add(coordinates.x);
    pChannel->add(coordinates.y);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;

    return result;
}


bool SandboxedHeuristicsSet::finishForCoordinates(unsigned int heuristic)
{
    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< FINISH_FOR_COORDINATES " << heuristic << endl;

    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context = _contexts[heuristic];

    std::ostringstream str;
	
	str << "Method: finishForCoordinates" << endl
        << "Parameters:" << endl
        << "    - Number of views: " << context.nb_views << endl
        << "    - ROI extent:      " << context.roi_extent << " pixels" << endl
        << "    - Sequence:        #" << context.sequence << endl
        << "    - Image size:      " << context.image_width << "x" << context.image_height << " pixels" << endl
        << "    - ROI position:    (" << context.coordinates.x << ", " << context.coordinates.y << ")" << endl;

	_strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_COORDINATES);
    pChannel->add(heuristic);
    pChannel->sendPacket();

    // Wait the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;

    return result;
}


bool SandboxedHeuristicsSet::computeSomeFeatures(unsigned int heuristic,
                                                 unsigned int nbFeatures,
                                                 unsigned int* indexes,
                                                 scalar_t* values)
{
    // Assertions
    assert(nbFeatures > 0);
    assert(indexes);
    assert(values);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< COMPUTE_SOME_FEATURES " << heuristic << " " << nbFeatures << endl;

    // Save the context (in case of crash)
    _currentHeuristic = heuristic;
    
    tContext context = _contexts[heuristic];

    std::ostringstream str;
	
	str << "Method: computeFeature" << endl
        << "Parameters:" << endl
        << "    - Number of views:    " << context.nb_views << endl
        << "    - ROI extent:         " << context.roi_extent << " pixels" << endl
        << "    - Sequence:           #" << context.sequence << endl
        << "    - Image size:         " << context.image_width << "x" << context.image_height << " pixels" << endl
        << "    - ROI position:       (" << context.coordinates.x << ", " << context.coordinates.y << ")" << endl;

    _strContext = str.str();

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_COMPUTE_SOME_FEATURES);
    pChannel->add(heuristic);
    pChannel->add(nbFeatures);
    pChannel->add((char*) indexes, nbFeatures * sizeof(unsigned int));
    pChannel->sendPacket();

    // Read the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    if (result)
        result = pChannel->read((char*) values, nbFeatures * sizeof(scalar_t));

    _lastError = (pChannel->getLastError() == ERROR_CHANNEL_SLAVE_CRASHED) ? ERROR_HEURISTIC_CRASHED : _lastError;

    return result;
}


bool SandboxedHeuristicsSet::reportStatistics(unsigned int heuristic,
                                              tHeuristicStatistics* statistics)
{
    // Assertions
    assert(statistics);

    if (getLastError() != ERROR_NONE)
        return false;

    _outStream << "< REPORT_STATISTICS " << heuristic << endl;

    // Send the command to the child
    CommunicationChannel* pChannel = _sandbox.channel();

    pChannel->startPacket(SANDBOX_COMMAND_HEURISTIC_REPORT_STATISTICS);
    pChannel->add(heuristic);
    pChannel->sendPacket();

    // Read the response
    bool result = pChannel->good();
    if (result)
        result = _sandbox.waitResponse(TIMEOUT_SANDBOX);

    if (result)
        result = pChannel->read((char*) statistics, sizeof(tHeuristicStatistics));

    return result;
}


tError SandboxedHeuristicsSet::getLastError()
{
    return (_lastError != ERROR_NONE ?
                _lastError :
                (_sandbox.getLastError() == ERROR_CHANNEL_SLAVE_CRASHED ?
                        ERROR_HEURISTIC_CRASHED : _sandbox.getLastError()));
}


tCommandProcessingResult SandboxedHeuristicsSet::processResponse(tSandboxMessage message)
{
    CommunicationChannel* pChannel = _sandbox.channel();

    tCommandProcessingResult result = COMMAND_UNKNOWN;

    if (message == SANDBOX_MESSAGE_CURRENT_HEURISTIC)
    {
        pChannel->read(&_currentHeuristic);
        result = COMMAND_PROCESSED;
        
        _outStream << "CURRENT HEURISTIC " << _currentHeuristic << endl;
    } 
    
    return result;
}
