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


/** @file   task_controller.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'TaskController' class
*/

#ifndef _TASK_CONTROLLER_H_
#define _TASK_CONTROLLER_H_

#include <mash-utils/declarations.h>
#include <mash-sandboxing/declarations.h>
#include <mash/features_computer.h>
#include <mash-instrumentation/instruments_set_interface.h>
#include <mash-network/server_listener.h>
#include <mash-network/client.h>
#include "notifier.h"


// Forward declaration
class Listener;


struct tTaskControllerConfiguration
{
    tTaskControllerConfiguration()
    : predictorSandboxConfiguration(0), heuristicsSandboxConfiguration(0),
      instrumentsSandboxConfiguration(0)
    {
    }
    
    std::string                     strPredictorsFolder;                ///< Path to the folder containing the predictor plugins
    std::string                     strHeuristicsFolder;                ///< Path to the folder containing the heuristic plugins
    std::string                     strInstrumentsFolder;               ///< Path to the folder containing the instrument plugins
    std::string                     strLogFolder;                       ///< Path to the folder to use for logging
    std::string                     strReportFolder;                    ///< Path to the folder of the data report
    std::string                     strCaptureFolder;                   ///< (goal-planning only) Path to the folder where the
                                                                        ///< images must be saved (empty to disable)
    Mash::tSandboxConfiguration*    predictorSandboxConfiguration;      ///< Configuration of the sandbox of the predictor (optional)
    Mash::tSandboxConfiguration*    heuristicsSandboxConfiguration;     ///< Configuration of the sandbox of the heuristics (optional)
    Mash::tSandboxConfiguration*    instrumentsSandboxConfiguration;    ///< Configuration of the sandbox of the instruments (optional)
};



//------------------------------------------------------------------------------
/// @brief  Base class for the Task Controllers
///
/// Used by the Experiment Server to run the experiments
//------------------------------------------------------------------------------
class TaskController
{
    //_____ Internal types ___________
public:
    struct tResult
    {
        tResult()
        : error(Mash::ERROR_NONE), strDetails("")
        {
        }

        tResult(Mash::tError e)
        : error(e), strDetails("")
        {
        }

        tResult(Mash::tError e, const std::string& details)
        : error(e), strDetails(details)
        {
        }
        
        Mash::tError error;
        std::string  strDetails;
    };
    
    
    //_____ Construction / Destruction __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Constructor
    //--------------------------------------------------------------------------
    TaskController(Listener* pListener);

    //--------------------------------------------------------------------------
    /// @brief  Destructor
    //--------------------------------------------------------------------------
    virtual ~TaskController();


    //_____ Methods to implement __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Setup the task controller
    ///
    /// @param  configuration   Configuration of the controller
    //--------------------------------------------------------------------------
    virtual Mash::tError setup(const tTaskControllerConfiguration& configuration) = 0;
    
    //--------------------------------------------------------------------------
    /// @brief  Set the global seed to use
    //--------------------------------------------------------------------------
    virtual void setGlobalSeed(unsigned int seed) = 0;

    //--------------------------------------------------------------------------
    /// @brief  Set the client (already connected to an Application Server) to
    ///         use
    //--------------------------------------------------------------------------
    virtual Mash::tError setClient(Mash::Client* pClient) = 0;

    //--------------------------------------------------------------------------
    /// @brief  Returns the client used
    //--------------------------------------------------------------------------
    virtual Mash::Client* getClient() = 0;

    //--------------------------------------------------------------------------
    /// @brief  Returns the Features Computer used
    //--------------------------------------------------------------------------
    virtual Mash::FeaturesComputer* getFeaturesComputer() = 0;
    
    //--------------------------------------------------------------------------
    /// @brief  Set the experiment parameters
    //--------------------------------------------------------------------------
    virtual tResult setParameters(const Mash::tExperimentParametersList& parameters) = 0;

    //--------------------------------------------------------------------------
    /// @brief  Load the specified predictor
    ///
    /// @param  strName             Name of the predictor
    /// @param  strModelFile        (Optional) Path to the model file to load
    /// @param  strInternalDataFile (Optional) Path to the internal data file to
    ///                             load
    /// @param  seed                (Optional) Seed for the predictor
    //--------------------------------------------------------------------------
    virtual tResult loadPredictor(const std::string strName,
                                  const std::string& strModelFile = "",
                                  const std::string& strInternalDataFile = "",
                                  unsigned int seed = 0) = 0;

    //--------------------------------------------------------------------------
    /// @brief  Set the predictor-specific parameters
    //--------------------------------------------------------------------------
    virtual bool setupPredictor(const Mash::tExperimentParametersList& parameters) = 0;

    //--------------------------------------------------------------------------
    /// @brief  Ask the predictor to train/learn
    //--------------------------------------------------------------------------
    virtual Mash::ServerListener::tAction train() = 0;

    //--------------------------------------------------------------------------
    /// @brief  Test the trained predictor
    //--------------------------------------------------------------------------
    virtual Mash::ServerListener::tAction test() = 0;

    //--------------------------------------------------------------------------
    /// @brief  Returns the list of features used by the predictor
    //--------------------------------------------------------------------------
    virtual bool getFeaturesUsed(Mash::tFeatureList &list) = 0;

    //--------------------------------------------------------------------------
    /// @brief  Save the predictor model
    //--------------------------------------------------------------------------
    virtual bool savePredictorModel() = 0;

    //--------------------------------------------------------------------------
    /// @brief  Ask the Task Controller to report the errors (if any)
    //--------------------------------------------------------------------------
    virtual Mash::ServerListener::tAction reportErrors() = 0;

    //--------------------------------------------------------------------------
    /// @brief  Returns the number of log files available
    //--------------------------------------------------------------------------
    virtual unsigned int getNbLogFiles();

    //--------------------------------------------------------------------------
    /// @brief  Return the content of a log file
    ///
    /// @param      index       Index of the log file
    /// @param[out] strName     Name of the log file
    /// @param[out] pBuffer     Buffer holding the content of the file
    /// @return                 The size of the buffer
    //--------------------------------------------------------------------------
    virtual int getLogFileContent(unsigned int index, std::string& strName,
                                  unsigned char** pBuffer, int64_t max_size = 0);

    //--------------------------------------------------------------------------
    /// @brief  Write task-specific data into the report data located at the
    ///         specified path
    //--------------------------------------------------------------------------
    virtual void fillReport(const std::string& strReportFolder) = 0;


    //_____ Methods __________
public:
    //--------------------------------------------------------------------------
    /// @brief  Indicates if the predictor is loaded
    //--------------------------------------------------------------------------
    inline bool isPredictorLoaded() const
    {
        return _bPredictorLoaded;
    }

    //--------------------------------------------------------------------------
    /// @brief  Load the specified instrument
    //--------------------------------------------------------------------------
    tResult loadInstrument(const std::string strName);

    //--------------------------------------------------------------------------
    /// @brief  Set the specific parameters of an instrument
    //--------------------------------------------------------------------------
    void setInstrumentParameters(const std::string strName,
                                 const Mash::tExperimentParametersList& parameters);

    //--------------------------------------------------------------------------
    /// @brief  Returns the Instruments Set used
    //--------------------------------------------------------------------------
    inline Mash::IInstrumentsSet* getInstrumentsSet()
    {
        return _pInstrumentsSet;
    }
    

protected:
    //--------------------------------------------------------------------------
    /// @brief  Create instances of the loaded instruments, and initialize them
    ///         using their specific parameters
    //--------------------------------------------------------------------------
    Mash::tError createInstruments();

    Mash::ServerListener::tAction sendHeuristicsErrorReport(Mash::FeaturesComputer* pFeaturesComputer,
                                                            Mash::tError heuristic_error);

    Mash::ServerListener::tAction sendInstrumentsErrorReport(Mash::tError instrument_error);

    Mash::ServerListener::tAction sendErrorReport(const std::string& strErrorType,
                                                  const Mash::ArgumentsList& args,
                                                  const std::string& strContext,
                                                  bool bHasStackTrace = false,
                                                  const std::string& strStackTrace = "");

    //--------------------------------------------------------------------------
    /// @brief  Process the features used by the predictor
    ///
    /// @param[out] bSuccess    Indicates if the processing was successful
    //--------------------------------------------------------------------------
    Mash::ServerListener::tAction processFeaturesUsed(bool &bSuccess);


    //_____ Attributes __________
protected:
    Listener*                                       _pListener;
    bool                                            _bPredictorLoaded;
    bool                                            _bInstrumentsCreated;
    Mash::IInstrumentsSet*                          _pInstrumentsSet;
    std::vector<Mash::tExperimentParametersList>    _instrumentsParameters;
    Notifier                                        _notifier;
};

#endif
