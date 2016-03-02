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


/** @file   sandboxed_heuristics.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'SandboxedHeuristics' class
*/

#include "sandboxed_heuristics.h"
#include <mash-sandboxing/sandbox_messages.h>
#include <mash-sandboxing/declarations.h>
#include <mash-utils/errors.h>
#include <sys/resource.h>
#include <memory.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    #include "dynlibs_manager_delegate.h"
#endif

using namespace std;
using namespace Mash;
using namespace Mash::SandboxTimeBudgetDeclarations;


/****************************** UTILITY FUNCTIONS *****************************/

inline void incrementTimeBudget(struct timeval* budget, const struct timeval &increment, unsigned multiplicator = 1)
{
    struct timeval result;

    struct timeval tmp;

    long long n = increment.tv_usec * multiplicator;

    tmp.tv_usec = n % 1000000;
    tmp.tv_sec = increment.tv_sec * multiplicator + (n - tmp.tv_usec) / 1000000;

    timeradd(budget, &tmp, &result);
    
    if (timercmp(&result, &BUDGET_MAXIMUM, >) == 0)
        *budget = result;
	else
        *budget = BUDGET_MAXIMUM;
}


inline bool decrementTimeBudget(struct timeval* budget, const struct timeval &elapsed)
{
    struct timeval result;

    timersub(budget, &elapsed, &result);
    *budget = result;
    
    return (result.tv_sec > 0) ||
           ((result.tv_sec == 0) && (result.tv_usec > 0));
}


inline void computeTimeout(const struct timeval &budget, struct timeval* timeout)
{
    timeradd(&budget, &ADDITIONAL_TIMEOUT, timeout);
}


inline void updateStatistics(tStatisticsEntry* statistics, const struct timeval &elapsed, bool incrementEventsCounter = true)
{
    struct timeval current = statistics->total_duration;
    
    timeradd(&current, &elapsed, &statistics->total_duration);
    
    if (incrementEventsCounter)
        ++statistics->nb_events;
}


inline void updateStatistics(tStatisticsComplexEntry* statistics, const struct timeval &elapsed, unsigned int nbSubEvents)
{
    struct timeval current = statistics->total_duration;
    
    timeradd(&current, &elapsed, &statistics->total_duration);
    
    if (nbSubEvents > 0)
    {
        ++statistics->nb_events;
        statistics->nb_subevents += nbSubEvents;
    }
}


inline void _updateStatistics(tStatisticsEntry* statistics, struct timeval* total)
{
    struct timeval current = *total;
    
    timeradd(&statistics->total_duration, &current, total);

    if (statistics->nb_events > 0)
    {
        div_t res = div((int) statistics->total_duration.tv_sec, (int) statistics->nb_events);
    
        statistics->mean_duration.tv_sec = res.quot;
        statistics->mean_duration.tv_usec = (res.rem * 1000000 + statistics->total_duration.tv_usec) / statistics->nb_events;
    }
}


inline void _updateStatistics(tStatisticsComplexEntry* statistics, struct timeval* total)
{
    struct timeval current = *total;
    
    timeradd(&statistics->total_duration, &current, total);

    if (statistics->nb_events > 0)
    {
        div_t res = div((int) statistics->total_duration.tv_sec, (int) statistics->nb_events);
    
        statistics->mean_event_duration.tv_sec = res.quot;
        statistics->mean_event_duration.tv_usec = (res.rem * 1000000 + statistics->total_duration.tv_usec) / statistics->nb_events;

        res = div((int) statistics->total_duration.tv_sec, (int) statistics->nb_subevents);
    
        statistics->mean_subevent_duration.tv_sec = res.quot;
        statistics->mean_subevent_duration.tv_usec = (res.rem * 1000000 + statistics->total_duration.tv_usec) / statistics->nb_subevents;
    }
}


inline void updateStatistics(tHeuristicStatistics* statistics)
{
    timerclear(&statistics->total_duration);
    
    _updateStatistics(&statistics->initialization,  &statistics->total_duration);
    _updateStatistics(&statistics->sequences,       &statistics->total_duration);
    _updateStatistics(&statistics->images,          &statistics->total_duration);
    _updateStatistics(&statistics->positions,       &statistics->total_duration);
    _updateStatistics(&statistics->features,        &statistics->total_duration);
}


/****************************** STATIC ATTRIBUTES *****************************/

SandboxedHeuristics::tCommandHandlersList SandboxedHeuristics::handlers;


/************************* CONSTRUCTION / DESTRUCTION *************************/

SandboxedHeuristics::SandboxedHeuristics(const CommunicationChannel& channel,
                                         OutStream* pOutStream)
: ISandboxedObject(channel, pOutStream), _pManager(0), _pLastImageReceived(0)
{
    timerclear(&_startTimestamp);
    timerclear(&_timeout);
    
    if (handlers.empty())
    {
        handlers[SANDBOX_COMMAND_HEURISTIC_SET_SEED]                = &SandboxedHeuristics::handleSetSeedCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_INIT]                    = &SandboxedHeuristics::handleInitCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_DIM]                     = &SandboxedHeuristics::handleDimCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_SEQUENCE]    = &SandboxedHeuristics::handlePrepareForSequenceCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_SEQUENCE]     = &SandboxedHeuristics::handleFinishForSequenceCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_IMAGE]       = &SandboxedHeuristics::handlePrepareForImageCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_IMAGE]        = &SandboxedHeuristics::handleFinishForImageCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_PREPARE_FOR_COORDINATES] = &SandboxedHeuristics::handlePrepareForCoordinatesCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_FINISH_FOR_COORDINATES]  = &SandboxedHeuristics::handleFinishForCoordinatesCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_COMPUTE_SOME_FEATURES]   = &SandboxedHeuristics::handleComputeSomeFeaturesCommand;
        handlers[SANDBOX_COMMAND_HEURISTIC_REPORT_STATISTICS]       = &SandboxedHeuristics::handleReportStatisticsCommand;
    }
    
    struct sigaction sa;
    sa.sa_handler = sigvtalrm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGALRM, &sa, 0);
}


SandboxedHeuristics::~SandboxedHeuristics()
{
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGALRM, &sa, 0);
    
    // Destroy the heuristics
    tHeuristicsIterator iter, iterEnd;
    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(); iter != iterEnd; ++iter)
    {
        setWardenContext(&iter->wardenContext);
        delete iter->pHeuristic;
        setWardenContext(0);
    }

    // Destroy the images
    tImagesIterator iter2, iterEnd2;
    for (iter2 = _images.begin(), iterEnd2 = _images.end(); iter2 != iterEnd2; ++iter2)
        delete iter2->first;

    // Destroy the heuristics manager
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    wardenEnableUnsafeFree();
#endif
    delete _pManager;
}


/******************** IMPLEMENTATION OF ISandboxedObject **********************/

tError SandboxedHeuristics::setPluginsFolder(const std::string& strPath)
{
    // Assertions
    assert(!strPath.empty());
    assert(!_pManager);
    
    _outStream << "Plugins folder: " << strPath << endl;
    
    // Create the heuristics manager
    _pManager = new HeuristicsManager(strPath);
    
    return ERROR_NONE;
}


tError SandboxedHeuristics::loadPlugin(const std::string& strName)
{
    // Assertions
    assert(!strName.empty());
    assert(_pManager);

    _outStream << "Loading plugin '" << strName << "'" << endl;

    // Initialize the information structure about the heuristic
    tHeuristicInfos infos;
    infos.pHeuristic                                = 0;
    infos.strName                                   = strName;
    infos.currentSeed                               = 0;
    infos.timeBudget                                = BUDGET_INITIALIZATION;

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    infos.wardenContext.sandboxed_object            = _heuristics.size();
    infos.wardenContext.memory_allocated            = 0;
    infos.wardenContext.memory_allocated_maximum    = 0;
    infos.wardenContext.memory_limit                = 200 * 1024 * 1024;
    infos.wardenContext.exceptions                  = WARDEN_EXCEPTION_DLOPEN;

    DynlibsManagerDelegate delegate(&infos.wardenContext);
    _pManager->setDelegate(&delegate);
#endif
    
    DYNLIB_HANDLE handle = _pManager->loadDynamicLibrary(strName);

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _pManager->setDelegate(0);
    infos.wardenContext.exceptions = 0;
#endif

    if (!handle)
    {
        string desc = _pManager->getLastErrorDescription();
        if (desc.empty())
            desc = getErrorDescription(_pManager->getLastError());
        
        _outStream << desc << endl;
        return _pManager->getLastError();
    }
    
    // Add it to the list of heuristics
    _heuristics.push_back(infos);
    
    return ERROR_NONE;
}


tError SandboxedHeuristics::createPlugins(Mash::OutStream* pOutStream,
                                          const std::vector<Mash::DataWriter>& dataWriters,
                                          const std::vector<Mash::DataWriter>& outCache,
                                          const std::vector<Mash::DataReader>& inCache,
                                          const Mash::PredictorModel& inModel,
                                          const Mash::DataReader& inInternalData,
                                          const Mash::PredictorModel& outModel,
                                          const Mash::DataWriter& outInternalData)
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());

    // Declarations
    struct timeval timeout;
    struct timeval elapsed;
    
    _outStream << "Creation of the objects defined in the plugins..." << endl;

    _channel.startPacket(SANDBOX_MESSAGE_KEEP_ALIVE);
    _channel.sendPacket();

    // Create the heuristics
    tHeuristicsIterator iter, iterEnd;
    unsigned int index = 0;
    for (iter = _heuristics.begin(), iterEnd = _heuristics.end(); iter != iterEnd; ++iter, ++index)
    {
        _outStream << "--- Creation of '" << iter->strName << "'" << endl;

        _channel.startPacket(SANDBOX_MESSAGE_CURRENT_HEURISTIC);
        _channel.add(index);
        _channel.sendPacket();

        computeTimeout(iter->timeBudget, &timeout);
        
#if MASH_PLATFORM == MASH_PLATFORM_LINUX
        DynlibsManagerDelegate delegate(&iter->wardenContext);
        _pManager->setDelegate(&delegate);
#endif

        startTimeCounter(timeout);
        
        iter->pHeuristic = _pManager->create(iter->strName);
        
        stopTimeCounter(&elapsed);

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
        _pManager->setDelegate(0);
#endif

        if (!iter->pHeuristic)
        {
            _outStream << getErrorDescription(_pManager->getLastError()) << endl;
            return _pManager->getLastError();
        }

        updateStatistics(&iter->statistics.initialization, elapsed);

        if (!decrementTimeBudget(&iter->timeBudget, elapsed))
        {
            updateStatistics(&iter->statistics);
            _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
            return ERROR_HEURISTIC_TIMEOUT;
        }

        _channel.startPacket(SANDBOX_MESSAGE_KEEP_ALIVE);
        _channel.sendPacket();
    }    
    
    return ERROR_NONE;
}


void SandboxedHeuristics::handleCommand(tSandboxMessage command)
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    assert(!handlers.empty());

    tCommandHandlersIterator iter = handlers.find(command);
    if (iter != handlers.end())
    {
        tCommandHandler handler = iter->second;
        tError result = (this->*handler)();
        
        if (result != ERROR_NONE)
        {
            _outStream << "< ERROR " << getErrorDescription(result) << endl;
            
            _channel.startPacket(SANDBOX_MESSAGE_ERROR);
            _channel.add(result);
            _channel.sendPacket();
        }
    }
    else
    {
        _channel.startPacket(SANDBOX_MESSAGE_UNKNOWN_COMMAND);
        _channel.sendPacket();
    }
}


/**************************** COMMAND HANDLERS ********************************/

tError SandboxedHeuristics::handleSetSeedCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());

    // Retrieve the heuristic index
    unsigned int heuristic;
    _channel.read(&heuristic);

    // Retrieve the seed
    unsigned int seed;
    _channel.read(&seed);
    
    if (!_channel.good())
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> SET_SEED " << heuristic << " " << seed << endl;
    
    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }
    
    // Set the seed of the heuristic
    _heuristics[heuristic].currentSeed = seed;

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handleInitCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    _channel.read(&heuristic);

    // Retrieve the number of views
    unsigned int nbViews;
    _channel.read(&nbViews);

    // Retrieve the ROI extent
    unsigned int roi_extent;
    _channel.read(&roi_extent);
    
    if (!_channel.good())
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> INIT " << heuristic << " " << nbViews << " " << roi_extent << endl;
    
    if ((heuristic >= _heuristics.size()) || (roi_extent == 0))
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }
    
    // Initialize the heuristic
    _heuristics[heuristic].pHeuristic->nb_views    = nbViews;
    _heuristics[heuristic].pHeuristic->roi_extent  = roi_extent;

    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    struct timeval timeout;
    struct timeval elapsed;

    computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

    startTimeCounter(timeout);
    setWardenContext(&_heuristics[heuristic].wardenContext);
    
    _heuristics[heuristic].pHeuristic->init();
    
    setWardenContext(0);
    stopTimeCounter(&elapsed);

    updateStatistics(&_heuristics[heuristic].statistics.initialization, elapsed, false);

    if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
    {
        updateStatistics(&_heuristics[heuristic].statistics);
        _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        return ERROR_HEURISTIC_TIMEOUT;
    }

    _heuristics[heuristic].currentSeed = rand();

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handleDimCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> DIM " << heuristic << endl;
    
    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }
    
    // Retrieve the dimension of the heuristic
    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    struct timeval timeout;
    struct timeval elapsed;

    computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

    startTimeCounter(timeout);
    setWardenContext(&_heuristics[heuristic].wardenContext);
    
    unsigned int dim = _heuristics[heuristic].pHeuristic->dim();
    
    setWardenContext(0);
    stopTimeCounter(&elapsed);

    updateStatistics(&_heuristics[heuristic].statistics.initialization, elapsed, false);

    if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
    {
        updateStatistics(&_heuristics[heuristic].statistics);
        _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        return ERROR_HEURISTIC_TIMEOUT;
    }

    _heuristics[heuristic].currentSeed = rand();

    _outStream << "< RESPONSE " << dim << endl;

    // Send the response
    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add(dim);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handlePrepareForSequenceCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> PREPARE_FOR_SEQUENCE " << heuristic << endl;
    
    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }
    
    // Tell the heuristic to prepare for a new sequence
    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    incrementTimeBudget(&_heuristics[heuristic].timeBudget, BUDGET_PER_SEQUENCE);
    
    struct timeval timeout;
    struct timeval elapsed;

    computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

    startTimeCounter(timeout);
    setWardenContext(&_heuristics[heuristic].wardenContext);
    
    _heuristics[heuristic].pHeuristic->prepareForSequence();
    
    setWardenContext(0);
    stopTimeCounter(&elapsed);

    updateStatistics(&_heuristics[heuristic].statistics.sequences, elapsed);

    if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
    {
        updateStatistics(&_heuristics[heuristic].statistics);
        _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        return ERROR_HEURISTIC_TIMEOUT;
    }

    _heuristics[heuristic].currentSeed = rand();

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handleFinishForSequenceCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> FINISH_FOR_SEQUENCE " << heuristic << endl;
    
    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }
    
    // Tell the heuristic that the sequence is finished
    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    struct timeval timeout;
    struct timeval elapsed;

    computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

    startTimeCounter(timeout);
    setWardenContext(&_heuristics[heuristic].wardenContext);
    
    _heuristics[heuristic].pHeuristic->finishForSequence();
    
    setWardenContext(0);
    stopTimeCounter(&elapsed);

    updateStatistics(&_heuristics[heuristic].statistics.sequences, elapsed, false);

    if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
    {
        updateStatistics(&_heuristics[heuristic].statistics);
        _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        return ERROR_HEURISTIC_TIMEOUT;
    }

    _heuristics[heuristic].currentSeed = rand();

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handlePrepareForImageCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());

    // Retrieve the heuristic index
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> PREPARE_FOR_IMAGE " << heuristic << endl;

    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }
    
    // Retrieve the image
    Heuristic* pHeuristic = _heuristics[heuristic].pHeuristic;

    if (pHeuristic->image)
    {
        tImagesIterator iter = _images.find(pHeuristic->image);
        
        iter->second--;
        if (iter->second == 0)
        {
            delete pHeuristic->image;
            _images.erase(iter);
        }
        
        pHeuristic->image = 0;
    }

    unsigned int width, height, view, pixelFormats;
    _channel.read((char*) &width, sizeof(unsigned int));

    if (width > 0)
    {
        if (_pLastImageReceived)
        {
            tImagesIterator iter = _images.find(_pLastImageReceived);

            iter->second--;
            if (iter->second == 0)
            {
                delete _pLastImageReceived;
                _images.erase(iter);
            }

            _pLastImageReceived = 0;
        }

        _channel.read((char*) &height, sizeof(unsigned int));
        _channel.read((char*) &view, sizeof(unsigned int));
        _channel.read((char*) &pixelFormats, sizeof(unsigned int));

        if (_channel.good())
        {
            pHeuristic->image = new Image(width, height, view);
            pHeuristic->image->addPixelFormats(pixelFormats);

            _pLastImageReceived = pHeuristic->image;
            _images[_pLastImageReceived] = 2;
        }

        if (_channel.good() && (pixelFormats & Image::PIXELFORMAT_RGB))
            _channel.read((char*) pHeuristic->image->rgbBuffer(), width * height * sizeof(RGBPixel_t));

        if (_channel.good() && (pixelFormats & Image::PIXELFORMAT_GRAY))
            _channel.read((char*) pHeuristic->image->grayBuffer(), width * height * sizeof(byte_t));
    }
    else
    {
        pHeuristic->image = _pLastImageReceived;
        _images[_pLastImageReceived] += 1;
        
        width = _pLastImageReceived->width();
        height = _pLastImageReceived->height();
    }

    if (!_channel.good())
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    // Tell the heuristic to prepare for the new image
    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    incrementTimeBudget(&_heuristics[heuristic].timeBudget, BUDGET_PER_PIXEL,
                        max(width * height, (unsigned int) (width * height * log(width * height))));
    
    struct timeval timeout;
    struct timeval elapsed;

    computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

    startTimeCounter(timeout);
    setWardenContext(&_heuristics[heuristic].wardenContext);
    
    pHeuristic->prepareForImage();
    
    setWardenContext(0);
    stopTimeCounter(&elapsed);

    updateStatistics(&_heuristics[heuristic].statistics.images, elapsed, width * height);

    if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
    {
        updateStatistics(&_heuristics[heuristic].statistics);
        _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        return ERROR_HEURISTIC_TIMEOUT;
    }

    _heuristics[heuristic].currentSeed = rand();

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handleFinishForImageCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }
    
    _outStream << "> FINISH_FOR_IMAGE " << heuristic << endl;

    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }
    
    // Tell the heuristic that working with this image is finished
    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);
    
    struct timeval timeout;
    struct timeval elapsed;

    computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

    startTimeCounter(timeout);
    setWardenContext(&_heuristics[heuristic].wardenContext);

    _heuristics[heuristic].pHeuristic->finishForImage();

    setWardenContext(0);
    stopTimeCounter(&elapsed);

    _heuristics[heuristic].currentSeed = rand();

    updateStatistics(&_heuristics[heuristic].statistics.images, elapsed, 0);

    if (_heuristics[heuristic].pHeuristic->image)
    {
        tImagesIterator iter = _images.find(_heuristics[heuristic].pHeuristic->image);
        
        iter->second--;
        if (iter->second == 0)
        {
            delete _heuristics[heuristic].pHeuristic->image;
            _images.erase(iter);
        }

        _heuristics[heuristic].pHeuristic->image = 0;
    }

    if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
    {
        updateStatistics(&_heuristics[heuristic].statistics);
        _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        return ERROR_HEURISTIC_TIMEOUT;
    }

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handlePrepareForCoordinatesCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    _channel.read(&heuristic);

    // Retrieve the coordinates
    coordinates_t coordinates;

    _channel.read(&coordinates.x);
    _channel.read(&coordinates.y);

    if (!_channel.good())
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> PREPARE_FOR_COORDINATES " << heuristic << " " << coordinates.x << " " << coordinates.y << endl;

    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }

    // Tell the heuristic to prepare for the new image
    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _heuristics[heuristic].pHeuristic->coordinates = coordinates;

    unsigned int roi_size = _heuristics[heuristic].pHeuristic->roi_extent * 2 + 1;

    incrementTimeBudget(&_heuristics[heuristic].timeBudget, BUDGET_PER_PIXEL,
                        max(roi_size * roi_size, (unsigned int) (roi_size * roi_size * log(roi_size * roi_size))));
    
    struct timeval timeout;
    struct timeval elapsed;

    computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

    startTimeCounter(timeout);
    setWardenContext(&_heuristics[heuristic].wardenContext);
    
    _heuristics[heuristic].pHeuristic->prepareForCoordinates();
    
    setWardenContext(0);
    stopTimeCounter(&elapsed);

    updateStatistics(&_heuristics[heuristic].statistics.positions, elapsed, roi_size * roi_size);

    if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
    {
        updateStatistics(&_heuristics[heuristic].statistics);
        _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        return ERROR_HEURISTIC_TIMEOUT;
    }

    _heuristics[heuristic].currentSeed = rand();

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handleFinishForCoordinatesCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> FINISH_FOR_COORDINATES " << heuristic << endl;

    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }

    // Tell the heuristic to prepare for the new image
    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    struct timeval timeout;
    struct timeval elapsed;

    computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

    startTimeCounter(timeout);
    setWardenContext(&_heuristics[heuristic].wardenContext);
    
    _heuristics[heuristic].pHeuristic->finishForCoordinates();
    
    setWardenContext(0);
    stopTimeCounter(&elapsed);

    updateStatistics(&_heuristics[heuristic].statistics.positions, elapsed, 0);

    if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
    {
        updateStatistics(&_heuristics[heuristic].statistics);
        _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        return ERROR_HEURISTIC_TIMEOUT;
    }

    _heuristics[heuristic].currentSeed = rand();

    memset(&_heuristics[heuristic].pHeuristic->coordinates, 0, sizeof(coordinates_t));

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


tError SandboxedHeuristics::handleComputeSomeFeaturesCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    _channel.read(&heuristic);

    // Retrieve the list of features to compute
    unsigned int    nbFeatures;
    unsigned int*   indexes = 0;
    scalar_t*       results = 0;

    _channel.read(&nbFeatures);

    if (!_channel.good())
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> COMPUTE_SOME_FEATURES " << heuristic << " " << nbFeatures << endl;

    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }

    if (nbFeatures == 0)
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }

    indexes = new unsigned int[nbFeatures];
    results = new scalar_t[nbFeatures];

    _channel.read((char*) indexes, nbFeatures * sizeof(unsigned int));

    if (!_channel.good())
    {
        delete[] indexes;
        delete[] results;

        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    // Tell the heuristic to prepare for the new image
    srand(_heuristics[heuristic].currentSeed);
    srand48(_heuristics[heuristic].currentSeed);

    _channel.startPacket(SANDBOX_MESSAGE_KEEP_ALIVE);
    _channel.sendPacket();

    unsigned int start_index = 0;

    struct timeval timeout;
    struct timeval elapsed;

    const unsigned int NB_FEATURES_PER_BATCH            = 100;
    const struct timeval BUDGET_PER_BATCH_OF_FEATURES   = { NB_FEATURES_PER_BATCH * BUDGET_PER_FEATURE.tv_sec, NB_FEATURES_PER_BATCH * BUDGET_PER_FEATURE.tv_usec };

    for (unsigned int i = 0; i < nbFeatures; ++i)
    {
        if (i == start_index)
        {
            incrementTimeBudget(&_heuristics[heuristic].timeBudget, BUDGET_PER_BATCH_OF_FEATURES);

            computeTimeout(_heuristics[heuristic].timeBudget, &timeout);

            startTimeCounter(timeout);
        }
        
        setWardenContext(&_heuristics[heuristic].wardenContext);
        
        results[i] = _heuristics[heuristic].pHeuristic->computeFeature(indexes[i]);

        setWardenContext(0);

        if (isnan(results[i]))
        {
            stopTimeCounter(&elapsed);

            updateStatistics(&_heuristics[heuristic].statistics.features, elapsed);

            delete[] indexes;
            delete[] results;

            updateStatistics(&_heuristics[heuristic].statistics);

            _outStream << getErrorDescription(ERROR_FEATURE_IS_NAN) << endl;
            return ERROR_FEATURE_IS_NAN;
        }

        if (i == start_index + NB_FEATURES_PER_BATCH - 1)
        {
            stopTimeCounter(&elapsed);

            updateStatistics(&_heuristics[heuristic].statistics.features, elapsed);

            if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
            {
                delete[] indexes;
                delete[] results;

                updateStatistics(&_heuristics[heuristic].statistics);

                _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
                return ERROR_HEURISTIC_TIMEOUT;
            }
        
            start_index += NB_FEATURES_PER_BATCH;
        }
    }

    if (start_index < nbFeatures)
    {
        stopTimeCounter(&elapsed);

        updateStatistics(&_heuristics[heuristic].statistics.features, elapsed);

        if (!decrementTimeBudget(&_heuristics[heuristic].timeBudget, elapsed))
        {
            delete[] indexes;
            delete[] results;

            updateStatistics(&_heuristics[heuristic].statistics);

            _outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
            return ERROR_HEURISTIC_TIMEOUT;
        }
    }

    _heuristics[heuristic].currentSeed = rand();

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add((char*) results, nbFeatures * sizeof(scalar_t));
    _channel.sendPacket();

    delete[] indexes;
    delete[] results;

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


Mash::tError SandboxedHeuristics::handleReportStatisticsCommand()
{
    // Assertions
    assert(_pManager);
    assert(!_heuristics.empty());
    
    // Retrieve the heuristic index
    unsigned int heuristic;
    if (!_channel.read(&heuristic))
    {
        _outStream << getErrorDescription(_channel.getLastError()) << endl;
        return _channel.getLastError();
    }

    _outStream << "> REPORT_STATISTICS " << heuristic << endl;

    if (heuristic >= _heuristics.size())
    {
        _outStream << getErrorDescription(ERROR_CHANNEL_PROTOCOL) << endl;
        return ERROR_CHANNEL_PROTOCOL;
    }

    updateStatistics(&_heuristics[heuristic].statistics);

    _channel.startPacket(SANDBOX_MESSAGE_RESPONSE);
    _channel.add((char*) &_heuristics[heuristic].statistics, sizeof(tHeuristicStatistics));

#if MASH_PLATFORM == MASH_PLATFORM_LINUX
    _channel.add((char*) &_heuristics[heuristic].wardenContext.memory_allocated_maximum, sizeof(size_t));
#else
    size_t fake = 0;
    _channel.add((char*) &fake, sizeof(size_t));
#endif

    _channel.sendPacket();

    return (_channel.good() ? ERROR_NONE : _channel.getLastError());
}


/*********************** TIME BUDGET-RELATED METHODS **************************/

void SandboxedHeuristics::startTimeCounter(const struct timeval &timeout)
{
    _timeout = timeout;
    
    setupAlarm(timeout);

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    timeradd(&usage.ru_utime, &usage.ru_stime, &_startTimestamp);
}


void SandboxedHeuristics::stopTimeCounter(struct timeval* elapsed)
{
    assert(elapsed);

    struct itimerval value;
    timerclear(&value.it_value);
    timerclear(&value.it_interval);
    
    setitimer(ITIMER_REAL, &value, 0);

    getElapsedTime(elapsed);
}


void SandboxedHeuristics::setupAlarm(const struct timeval &timeout)
{
    assert((timeout.tv_sec > 0) || (timeout.tv_usec > 0));
    
    struct itimerval value;
    timerclear(&value.it_interval);
    
    value.it_value = timeout;
    
    setitimer(ITIMER_REAL, &value, 0);
}


void SandboxedHeuristics::getElapsedTime(struct timeval* elapsed)
{
    assert(elapsed);

    struct timeval current;
    
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    timeradd(&usage.ru_utime, &usage.ru_stime, &current);

    timersub(&current, &_startTimestamp, elapsed);
}


void SandboxedHeuristics::sigvtalrm_handler(int s)
{
    assert(s == SIGALRM);
    assert(pInstance);
    assert(dynamic_cast<SandboxedHeuristics*>(pInstance));
    assert(timerisset(&dynamic_cast<SandboxedHeuristics*>(pInstance)->_timeout));
    
    SandboxedHeuristics* pInstance2 = dynamic_cast<SandboxedHeuristics*>(pInstance);
    
    struct timeval elapsed;
    pInstance2->getElapsedTime(&elapsed);
    
	if (timercmp(&elapsed, &pInstance2->_timeout, >=) != 0)
    {
        // Simulate the logged messages when the time budget is exhausted
        pInstance2->_outStream << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        pInstance2->_outStream << "< ERROR " << getErrorDescription(ERROR_HEURISTIC_TIMEOUT) << endl;
        
        pInstance2->_channel.startPacket(SANDBOX_MESSAGE_ERROR);
        pInstance2->_channel.add(ERROR_HEURISTIC_TIMEOUT);
        pInstance2->_channel.sendPacket();
        
        exit(1);
    }

    pInstance2->_channel.startPacket(SANDBOX_MESSAGE_KEEP_ALIVE);
    pInstance2->_channel.sendPacket();
    
    struct timeval remaining;
    timersub(&pInstance2->_timeout, &elapsed, &remaining);
    pInstance2->setupAlarm(remaining);
}
