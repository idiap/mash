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


/** @file   sandboxed_heuristics.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxedHeuristics' class
*/

#ifndef _SANDBOXEDHEURISTICS_H_
#define _SANDBOXEDHEURISTICS_H_

#include "sandboxed_object.h"
#include <mash-sandboxing/declarations.h>
#include <mash/heuristics_manager.h>
#include <mash/heuristic.h>
#include <sys/time.h>
#include <vector>


//------------------------------------------------------------------------------
/// @brief  Represents a group of sandboxed heuristics
//------------------------------------------------------------------------------
class SandboxedHeuristics: public ISandboxedObject
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    //--------------------------------------------------------------------------
    SandboxedHeuristics(const Mash::CommunicationChannel& channel,
                        Mash::OutStream* pOutStream);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~SandboxedHeuristics();


    //_____ Implementation of ISandboxedObject __________
public:
    virtual Mash::tError setPluginsFolder(const std::string& strPath);
    virtual Mash::tError loadPlugin(const std::string& strName);
    virtual Mash::tError createPlugins(Mash::OutStream* pOutStream,
                                       const std::vector<Mash::DataWriter>& dataWriters,
                                       const std::vector<Mash::DataWriter>& outCache,
                                       const std::vector<Mash::DataReader>& inCache,
                                       const Mash::PredictorModel& inModel,
                                       const Mash::DataReader& inInternalData,
                                       const Mash::PredictorModel& outModel,
                                       const Mash::DataWriter& outInternalData);
    virtual void handleCommand(Mash::tSandboxMessage command);


    //_____ Command handlers __________
private:
    Mash::tError handleSetSeedCommand();
    Mash::tError handleInitCommand();
    Mash::tError handleDimCommand();
    Mash::tError handlePrepareForSequenceCommand();
    Mash::tError handleFinishForSequenceCommand();
    Mash::tError handlePrepareForImageCommand();
    Mash::tError handleFinishForImageCommand();
    Mash::tError handlePrepareForCoordinatesCommand();
    Mash::tError handleFinishForCoordinatesCommand();
    Mash::tError handleComputeSomeFeaturesCommand();
    Mash::tError handleReportStatisticsCommand();


    //_____ Time budget-related methods __________
protected:
    void startTimeCounter(const struct timeval &timeout);
    void stopTimeCounter(struct timeval* elapsed);
    void setupAlarm(const struct timeval &timeout);
    void getElapsedTime(struct timeval* elapsed);

    static void sigvtalrm_handler(int s);


    //_____ Internal types __________
protected:
    struct tHeuristicInfos
    {
        Mash::Heuristic*            pHeuristic;
        std::string                 strName;
        unsigned int                currentSeed;
        struct timeval              timeBudget;
        Mash::tHeuristicStatistics  statistics;
        tWardenContext              wardenContext;
    };
    
    typedef std::vector<tHeuristicInfos>    tHeuristicsList;
    typedef tHeuristicsList::iterator       tHeuristicsIterator;
    
    typedef Mash::tError (SandboxedHeuristics::*tCommandHandler)();

    typedef std::map<Mash::tSandboxMessage, tCommandHandler>    tCommandHandlersList;
    typedef tCommandHandlersList::iterator                      tCommandHandlersIterator;

    typedef std::map<Mash::Image*, unsigned int>    tImagesList;
    typedef tImagesList::iterator                   tImagesIterator;

    
    //_____ Attributes __________
protected:
    static tCommandHandlersList handlers;

    Mash::HeuristicsManager*    _pManager;
    tHeuristicsList             _heuristics;
    struct timeval              _startTimestamp;
    struct timeval              _timeout;
    tImagesList                 _images;
    Mash::Image*                _pLastImageReceived;
};

#endif
