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


/** @file   listener.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declaration of the 'Listener' class
*/

#ifndef _LISTENER_H_
#define _LISTENER_H_

#include "task_controller.h"
#include <mash-network/server_listener.h>
#include <tinyxml/tinyxml.h>
#include <map>


enum tSandboxingMechanism
{
    SANDBOXING_HEURISTICS  = 1,
    SANDBOXING_PREDICTOR   = 2,
    SANDBOXING_INSTRUMENTS = 4,
};


struct tListenerConfiguration
{
    tListenerConfiguration()
    : strHost(""), port(10000), bStandalone(false), strScriptsDir(""), strOutputDir("out/"), verbosity(0),
      bInFrameworkBuildDir(false), strCaptureDir(""), bNoCompilation(false), strRepository("heuristics.git"),
      strHeuristicsDir("heuristics/"), strBuildDir("build/"), strClassifiersDir("classifiers/"),
      strPlannersDir("goalplanners/"), strInstrumentsDir("instruments/"),
      sandboxingMechanisms(SANDBOXING_HEURISTICS | SANDBOXING_PREDICTOR | SANDBOXING_INSTRUMENTS),
      strCoreDumpTemplate(""), strSandboxUsername(""), strSandboxJailDir("jail"), strSandboxScriptsDir(""),
      strSandboxTempDir("./")
    {
    }
    
    // General
    std::string     strHost;                ///< The host name or IP address that the server must listen on
    unsigned int    port;                   ///< The port that the server must listen on
    bool            bStandalone;            ///< Indicates if we are running in 'standalone mode' or in 'server mode'        
    std::string     strScriptsDir;          ///< The directory in which the 'manage.py' script is located
    std::string     strOutputDir;           ///< The directory into which the data files must be written by the instruments
    unsigned int    verbosity;              ///< Level of verbosity
    bool            bInFrameworkBuildDir;   ///< Indicates if the Experiment Server is executed from its build directory

    // Goal-planning
    std::string     strCaptureDir;          ///< (Standalone only) The directory into which the received images must be written

    // Heuristics
    bool            bNoCompilation;         ///< Indicates if the heuristics are already compiled
    std::string     strRepository;          ///< Path to the cloned repository of heuristics
    std::string     strHeuristicsDir;       ///< The directory in which the compiled heuristics are located
    std::string     strBuildDir;            ///< The directory used to build the heuristics

    // Predictors
    std::string     strClassifiersDir;      ///< The directory in which the compiled classifiers are located
    std::string     strPlannersDir;         ///< The directory in which the compiled planners are located
    std::string     strPredictorModel;      ///< The model file that the predictor must load
    std::string     strPredictorData;       ///< The internal data to provide to the predictor alongside the model to load

    // Instruments
    std::string     strInstrumentsDir;      ///< The directory in which the compiled instruments are located

    // Sandboxing
    unsigned char   sandboxingMechanisms;   ///< Indicates which sandboxing mechanisms are enabled
    std::string     strCoreDumpTemplate;    ///< Template of the name of the core dump files
    std::string     strSandboxUsername;     ///< User for the untrusted plugins in the sandboxes
    std::string     strSandboxJailDir;      ///< Parent folder for the jail ones
    std::string     strSandboxScriptsDir;   ///< The directory in which the 'coredump_analyzer.py' script is located
    std::string     strSandboxTempDir;      ///< The temporary directory for the sandboxes
    std::string     strSourceHeuristics;    ///< Directory containing the source code of the heuristics
    std::string     strSourceClassifiers;   ///< Directory containing the source code of the classifiers
    std::string     strSourcePlanners;      ///< Directory containing the source code of the goal-planners
    std::string     strSourceInstruments;   ///< Directory containing the source code of the instruments
};


class Listener: public Mash::ServerListener
{
    //_____ Internal types __________
public:
    enum tTask
    {
        TASK_NONE,
        TASK_CLASSIFICATION,
        TASK_GOALPLANNING,
    };


    //_____ Construction / Destruction __________
public:
    Listener(int socket);
    virtual ~Listener();


    //_____ Implementation of ServerListener __________
public:
    virtual tAction handleCommand(const std::string& strCommand,
                                  const Mash::ArgumentsList& arguments);
                               
    bool sendResponse(const std::string& strResponse,
                      const Mash::ArgumentsList& arguments);

    bool sendData(const unsigned char* data, int size);


    //_____ Methods __________
public:
    inline bool failure() const { return _error; }
    inline tTask task() const { return _task; }


    //_____ Static methods __________
public:
    static void initialize(const tListenerConfiguration& configuration);
    static ServerListener* createListener(int socket);


    //_____ Command handling __________
private:
    // Common commands
    tAction handleStatusCommand(const Mash::ArgumentsList& arguments);
    tAction handleInfoCommand(const Mash::ArgumentsList& arguments);
    tAction handleDoneCommand(const Mash::ArgumentsList& arguments);
    tAction handleLogsCommand(const Mash::ArgumentsList& arguments);
    tAction handleSleepCommand(const Mash::ArgumentsList& arguments);
    
    // Experiment setup
    tAction handleResetCommand(const Mash::ArgumentsList& arguments);
    tAction handleSetExperimentTypeCommand(const Mash::ArgumentsList& arguments);
    tAction handleUseApplicationServerCommand(const Mash::ArgumentsList& arguments);
    tAction handleUseGlobalSeedCommand(const Mash::ArgumentsList& arguments);
    tAction handleBeginExperimentSetupCommand(const Mash::ArgumentsList& arguments);
    tAction handleEndExperimentSetupCommand(const Mash::ArgumentsList& arguments);

    // Instruments
    tAction handleUseInstrumentCommand(const Mash::ArgumentsList& arguments);
    tAction handleBeginInstrumentSetupCommand(const Mash::ArgumentsList& arguments);
    tAction handleEndInstrumentSetupCommand(const Mash::ArgumentsList& arguments);

    // Predictor
    tAction handleUsePredictorModel(const Mash::ArgumentsList& arguments);
    tAction handleUsePredictorInternalData(const Mash::ArgumentsList& arguments);
    tAction handleUsePredictorCommand(const Mash::ArgumentsList& arguments);
    tAction handleBeginPredictorSetupCommand(const Mash::ArgumentsList& arguments);
    tAction handleEndPredictorSetupCommand(const Mash::ArgumentsList& arguments);

    // Heuristics
    tAction handleUseHeuristicsRepositoryCommand(const Mash::ArgumentsList& arguments);
    tAction handleUseHeuristicCommand(const Mash::ArgumentsList& arguments);

    // Experiment
    tAction handleTrainPredictorCommand(const Mash::ArgumentsList& arguments);
    tAction handleTestPredictorCommand(const Mash::ArgumentsList& arguments);

    // Report
    tAction handleReportDataCommand(const Mash::ArgumentsList& arguments);
    tAction handleReportErrorsCommand(const Mash::ArgumentsList& arguments);


    //_____ Utility methods __________
private:
    Mash::tError setupTaskController();
    void generateReports();


    //_____ Internal types __________
private:
    typedef tAction (Listener::*tCommandHandler)(const Mash::ArgumentsList&);
    
    typedef std::map<std::string, tCommandHandler>  tCommandHandlersList;
    typedef tCommandHandlersList::iterator          tCommandHandlersIterator;

    enum tMode
    {
        MODE_NORMAL,
        MODE_EXPERIMENT_SETUP,
        MODE_INSTRUMENT_SETUP,
        MODE_PREDICTOR_SETUP,
    };


    //_____ Attributes __________
private:
    static tCommandHandlersList     handlers;
    static tListenerConfiguration   configuration;

    bool                            _bGlobalSeedSelected;
    tTask                           _task;
    TaskController*                 _taskController;
    tMode                           _mode;
    Mash::tExperimentParametersList _experimentParameters;
    Mash::tExperimentParametersList _predictorParameters;
    std::string                     _strCurrentInstrument;
    Mash::tExperimentParametersList _instrumentParameters;
    Mash::PredictorModel            _predictorModel;
    bool                            _bTrainingDone;
    bool                            _error;
    Mash::OutStream                 _clientStream;
    TiXmlDocument                   _xmlReportDocument;
    bool                            _bExperimentDone;
    bool                            _bReportsGenerated;
};

#endif
