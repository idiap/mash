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


/** @file   sandboxed_instruments.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'SandboxedInstruments' class
*/

#ifndef _SANDBOXEDINSTRUMENTS_H_
#define _SANDBOXEDINSTRUMENTS_H_

#include "sandboxed_object.h"
#include "sandbox_input_set.h"
#include "sandbox_task.h"
#include <mash-sandboxing/declarations.h>
#include <mash-instrumentation/instruments_manager.h>
#include <mash-instrumentation/instrument.h>
#include <vector>


//------------------------------------------------------------------------------
/// @brief  Represents a group of sandboxed instruments
//------------------------------------------------------------------------------
class SandboxedInstruments: public ISandboxedObject
{
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    //--------------------------------------------------------------------------
    SandboxedInstruments(const Mash::CommunicationChannel& channel,
                         Mash::OutStream* pOutStream);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~SandboxedInstruments();


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


    //_____ Methods __________
public:
    inline unsigned int nbInstruments() const
    {
        return (unsigned int) _instruments.size();
    }

    inline std::string instrumentName(unsigned int index) const
    {
        return _instruments[index].strName;
    }


    //_____ Event handlers __________
private:
    Mash::tError handleInstrumentSetupCommand();

    Mash::tError handleExperimentDoneEvent();

    Mash::tError handleClassificationExperimentStartedEvent();
    Mash::tError handleClassifierTrainingStartedEvent();
    Mash::tError handleClassifierTrainingDoneEvent();
    Mash::tError handleClassifierTestStartedEvent();
    Mash::tError handleClassifierTestDoneEvent();
    Mash::tError handleClassifierClassificationDoneEvent();
    Mash::tError handleFeaturesComputedByClassifierEvent();

    Mash::tError handleGoalplanningExperimentStartedEvent();
    Mash::tError handlePlannerLearningStartedEvent();
    Mash::tError handlePlannerLearningDoneEvent();
    Mash::tError handlePlannerTestStartedEvent();
    Mash::tError handlePlannerTestDoneEvent();
    Mash::tError handlePlannerActionChoosenEvent();
    Mash::tError handleFeaturesComputedByPlannerEvent();

    Mash::tError handleFeatureListReportedEvent();


    //_____ Internal types __________
protected:
    struct tInstrumentInfos
    {
        Mash::Instrument*   pInstrument;
        std::string         strName;
        tWardenContext      wardenContext;
    };
    
    typedef std::vector<tInstrumentInfos>   tInstrumentsList;
    typedef tInstrumentsList::iterator      tInstrumentsIterator;
    
    typedef Mash::tError (SandboxedInstruments::*tEventHandler)();

    typedef std::map<Mash::tSandboxMessage, tEventHandler>  tEventHandlersList;
    typedef tEventHandlersList::iterator                    tEventHandlersIterator;

    
    //_____ Attributes __________
protected:
    static tEventHandlersList handlers;

    Mash::InstrumentsManager*   _pManager;
    tInstrumentsList            _instruments;
    SandboxInputSet             _inputSet;
    SandboxTask                 _task;
};

#endif
