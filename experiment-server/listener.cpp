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


/** @file   listener.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Implementation of the 'Listener' class
*/

#include "listener.h"
#include "classification_task.h"
#include "goalplanning_task.h"
#include <mash-utils/errors.h>
#include <mash/heuristics_manager.h>
#include <mash-network/client.h>
#include <mash-network/server.h>
#include <mash-utils/stringutils.h>
#include <mash/sandboxed_heuristics_set.h>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <sys/stat.h> 
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>


using namespace std;
using namespace Mash;


/****************************** STATIC ATTRIBUTES *****************************/

Listener::tCommandHandlersList  Listener::handlers;
tListenerConfiguration          Listener::configuration;


/****************************** UTILITY FUNCTIONS *****************************/

std::string checkPaths(const std::string& strPaths)
{
    tStringList paths = StringUtils::split(strPaths, ";");
    
    string result = "";
    
    tStringList::iterator iter, iterEnd;
    for (iter = paths.begin(), iterEnd = paths.end(); iter != iterEnd; ++iter)
    {
        if (!result.empty())
            result += ";";

        result += *iter;

        if (iter->at(iter->length() - 1) != '/')
            result += "/";
    }

    return result;
}


void makedirs(const std::string& strPath)
{
    size_t offset = strPath.find_last_of("/");
    if (offset != string::npos)
    {
        string dest = strPath.substr(0, offset + 1);
        
        size_t start = dest.find("/", 0);
        while (start != string::npos)
        {
            string dirname = dest.substr(0, start);
                        
            DIR* d = opendir(dirname.c_str());
            if (!d)
                mkdir(dirname.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            else
                closedir(d);

            start = dest.find("/", start + 1);
        }
    }
}


void removeDirectoryContent(const std::string& strPath)
{
    DIR*            dir;
    struct dirent*  entry;

    if ((strPath == ".") || (strPath == "..") || (strPath == "./") || (strPath == "../"))
        return;

    dir = opendir(strPath.c_str());
    if (dir == NULL)
        return;

    while ((entry = readdir(dir)) != NULL)
    {
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
        {
            string path;
            
            if (strPath.at(strPath.size() - 1) == '/')
                path = strPath + entry->d_name;
            else
                path = strPath + "/" + entry->d_name;
            
            if (entry->d_type == DT_DIR)
                removeDirectoryContent(path.c_str());

            remove(path.c_str());
        }
    }

    closedir(dir);
}


/************************* CONSTRUCTION / DESTRUCTION *************************/

Listener::Listener(int socket)
: ServerListener(socket), _bGlobalSeedSelected(false), _task(TASK_NONE),
  _taskController(0), _mode(MODE_NORMAL), _bTrainingDone(false), _error(false),
  _bExperimentDone(false), _bReportsGenerated(false)
{
    char buffer1[50];
    char buffer2[50];

    sprintf(buffer1, "Listener #%d", socket);
    sprintf(buffer2, "listener_%d_$TIMESTAMP.log", socket);

    _outStream.setVerbosityLevel(1);
    _outStream.open(buffer1, Server::strLogFolder + buffer2, 200 * 1024);

    sprintf(buffer1, "Client #%d", socket);
    sprintf(buffer2, "client_%d_$TIMESTAMP.log", socket);

    _clientStream.setVerbosityLevel(2);
    _clientStream.open(buffer1, Server::strLogFolder + buffer2, 200 * 1024);

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	_xmlReportDocument.LinkEndChild(decl);

    if (!Listener::configuration.strPredictorModel.empty())
        _predictorModel.open(Listener::configuration.strPredictorModel);
}


Listener::~Listener()
{
    Client* pClient = (_taskController ? _taskController->getClient() : 0);

    delete _taskController;
    delete pClient;

    _clientStream.deleteFile();
}


/********************** IMPLEMENTATION OF ServerListener **********************/

ServerListener::tAction Listener::handleCommand(const std::string& strCommand,
                                                const ArgumentsList& arguments)
{
    // In standalone mode, log the command (already done in server mode)
    if (configuration.bStandalone)
    {
        cout << "> " << strCommand;
        
        for (int i = 0; i < arguments.size(); ++i)
            cout << " " << arguments.getString(i);
        
        cout << endl;
    }    

    if ((_mode == MODE_NORMAL) ||
        ((_mode == MODE_EXPERIMENT_SETUP) && (strCommand == "END_EXPERIMENT_SETUP")) ||
        ((_mode == MODE_INSTRUMENT_SETUP) && (strCommand == "END_INSTRUMENT_SETUP")) ||
        ((_mode == MODE_PREDICTOR_SETUP) && (strCommand == "END_PREDICTOR_SETUP")))
    {    
        tCommandHandlersIterator iter = handlers.find(strCommand);
        if (iter != handlers.end())
        {
            tCommandHandler handler = iter->second;
            return (this->*handler)(arguments);
        }

        if (!sendResponse("UNKNOWN_COMMAND", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;
    }
    else if (_mode == MODE_EXPERIMENT_SETUP)
    {
        _experimentParameters[strCommand] = arguments;

        if  (!sendResponse("OK", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;
    }
    else if (_mode == MODE_INSTRUMENT_SETUP)
    {
        _instrumentParameters[strCommand] = arguments;

        if  (!sendResponse("OK", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;
    }
    else if (_mode == MODE_PREDICTOR_SETUP)
    {
        _predictorParameters[strCommand] = arguments;

        if  (!sendResponse("OK", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;
    }
    else
    {
        // We should never get there
        assert(false);
    }

    return ACTION_NONE;
}


bool Listener::sendResponse(const std::string& strResponse,
                            const ArgumentsList& arguments)
{
    // Test if we are in server or standalone mode
    if (!configuration.bStandalone)
        return ServerListener::sendResponse(strResponse, arguments);


    // Write the response to the output stream, but filter the notifications
    // for verbosity levels < 2
    if ((configuration.verbosity == 1) && (strResponse == "NOTIFICATION"))
        _outStream.setVerbosityLevel(2);

    _outStream << "< " << strResponse;

    for (int i = 0; i < arguments.size(); ++i)
        _outStream << " " << arguments.getString(i);

    _outStream << endl;

    if ((configuration.verbosity == 1) && (strResponse == "NOTIFICATION"))
        _outStream.setVerbosityLevel(1);

    
    // Write the response to the standard output if the verbosity level isn't
    // high enough to let the output stream do it: we are in standalone mode
    // and we'd like to know what is happening
    if (configuration.verbosity == 0)
    {
        // Filter the notifications
        if (strResponse != "NOTIFICATION")
        {
            cout << "< " << strResponse;
    
            for (int i = 0; i < arguments.size(); ++i)
                cout << " " << arguments.getString(i);
    
            cout << endl;
        }
    }
    
    _error = (strResponse == "ERROR") ||
             (strResponse == "INVALID_ARGUMENTS") ||
             (strResponse == "FAILED_TO_CONNECT") ||
             (strResponse == "APPLICATION_SERVER_BUSY") ||
             (strResponse == "INVALID_APPLICATION_SERVER");
    
    return true;
}


bool Listener::sendData(const unsigned char* data, int size)
{
    // Test if we are in server or standalone mode
    if (!configuration.bStandalone)
        return ServerListener::sendData(data, size);

    // Write the data to the output stream if necessary
    if (configuration.verbosity == 0)
    {
        cout << "< ";
        cout.write((const char*) data, size);
        cout << endl;
    }
    else
    {
        _outStream << "< ";
        _outStream.write((char*) data, size);
        _outStream << endl;
    }
    
    return true;
}


/******************************* STATIC METHODS *******************************/

void Listener::initialize(const tListenerConfiguration& configuration)
{
    // Common commands
    handlers["STATUS"] =                        &Listener::handleStatusCommand;
    handlers["INFO"] =                          &Listener::handleInfoCommand;
    handlers["DONE"] =                          &Listener::handleDoneCommand;
    handlers["LOGS"] =                          &Listener::handleLogsCommand;
    handlers["SLEEP"] =                         &Listener::handleSleepCommand;
    
    // Experiment setup
    handlers["RESET"] =                         &Listener::handleResetCommand;
    handlers["SET_EXPERIMENT_TYPE"] =           &Listener::handleSetExperimentTypeCommand;
    handlers["USE_APPLICATION_SERVER"] =        &Listener::handleUseApplicationServerCommand;
    handlers["USE_GLOBAL_SEED"] =               &Listener::handleUseGlobalSeedCommand;
    handlers["BEGIN_EXPERIMENT_SETUP"] =        &Listener::handleBeginExperimentSetupCommand;
    handlers["END_EXPERIMENT_SETUP"] =          &Listener::handleEndExperimentSetupCommand;

    // Instruments
    handlers["USE_INSTRUMENT"] =                &Listener::handleUseInstrumentCommand;
    handlers["BEGIN_INSTRUMENT_SETUP"] =        &Listener::handleBeginInstrumentSetupCommand;
    handlers["END_INSTRUMENT_SETUP"] =          &Listener::handleEndInstrumentSetupCommand;

    // Predictor
    handlers["USE_PREDICTOR_MODEL"] =           &Listener::handleUsePredictorModel;
    handlers["USE_PREDICTOR_INTERNAL_DATA"] =   &Listener::handleUsePredictorInternalData;
    handlers["USE_PREDICTOR"] =                 &Listener::handleUsePredictorCommand;
    handlers["BEGIN_PREDICTOR_SETUP"] =         &Listener::handleBeginPredictorSetupCommand;
    handlers["END_PREDICTOR_SETUP"] =           &Listener::handleEndPredictorSetupCommand;

    // Heuristics
    handlers["USE_HEURISTICS_REPOSITORY"] =     &Listener::handleUseHeuristicsRepositoryCommand;
    handlers["USE_HEURISTIC"] =                 &Listener::handleUseHeuristicCommand;

    // Experiment
    handlers["TRAIN_PREDICTOR"] =               &Listener::handleTrainPredictorCommand;
    handlers["TEST_PREDICTOR"] =                &Listener::handleTestPredictorCommand;

    // Report
    handlers["REPORT_DATA"] =                   &Listener::handleReportDataCommand;
    handlers["REPORT_ERRORS"] =                 &Listener::handleReportErrorsCommand;
    
    Listener::configuration = configuration;
    
    if (Listener::configuration.strScriptsDir.length() == 0)
    {
        if (Listener::configuration.bInFrameworkBuildDir)
            Listener::configuration.strScriptsDir = MASH_SOURCE_DIR "experiment-server/";
        else
            Listener::configuration.strScriptsDir = "./";
    }
    else if (Listener::configuration.strScriptsDir.at(Listener::configuration.strScriptsDir.length() - 1) != '/')
    {
        Listener::configuration.strScriptsDir += "/";
    }

    if (Listener::configuration.strOutputDir.length() == 0)
        Listener::configuration.strOutputDir = "out/";
    else if (Listener::configuration.strOutputDir.at(Listener::configuration.strOutputDir.length() - 1) != '/')
        Listener::configuration.strOutputDir += "/";
        
    makedirs(Listener::configuration.strOutputDir);
        
    removeDirectoryContent(Listener::configuration.strOutputDir);

    if (Listener::configuration.strCaptureDir.length() > 0)
    {
        if (Listener::configuration.strCaptureDir.at(Listener::configuration.strCaptureDir.length() - 1) != '/')
            Listener::configuration.strCaptureDir += "/";

        removeDirectoryContent(Listener::configuration.strCaptureDir);
    }

    if (Listener::configuration.strRepository.length() == 0)
        Listener::configuration.strRepository = "heuristics.git";
    else if (Listener::configuration.strRepository.at(Listener::configuration.strRepository.length() - 1) == '/')
        Listener::configuration.strRepository = Listener::configuration.strRepository.substr(0, Listener::configuration.strRepository.length() - 1);

    if (Listener::configuration.strHeuristicsDir.length() == 0)
        Listener::configuration.strHeuristicsDir = "heuristics/";
    else if (Listener::configuration.strHeuristicsDir.at(Listener::configuration.strHeuristicsDir.length() - 1) != '/')
        Listener::configuration.strHeuristicsDir += "/";

    if (Listener::configuration.strBuildDir.length() == 0)
        Listener::configuration.strBuildDir = "build/";
    else if (Listener::configuration.strBuildDir.at(Listener::configuration.strBuildDir.length() - 1) != '/')
        Listener::configuration.strBuildDir += "/";
    
    if (Listener::configuration.strClassifiersDir.length() == 0)
        Listener::configuration.strClassifiersDir = "classifiers/";
    else if (Listener::configuration.strClassifiersDir.at(Listener::configuration.strClassifiersDir.length() - 1) != '/')
        Listener::configuration.strClassifiersDir += "/";

    if (Listener::configuration.strPlannersDir.length() == 0)
        Listener::configuration.strPlannersDir = "goalplanners/";
    else if (Listener::configuration.strPlannersDir.at(Listener::configuration.strPlannersDir.length() - 1) != '/')
        Listener::configuration.strPlannersDir += "/";

    if (Listener::configuration.strInstrumentsDir.length() == 0)
        Listener::configuration.strInstrumentsDir = "instruments/";
    else if (Listener::configuration.strInstrumentsDir.at(Listener::configuration.strInstrumentsDir.length() - 1) != '/')
        Listener::configuration.strInstrumentsDir += "/";

    if (configuration.sandboxingMechanisms != 0)
    {
        if (Listener::configuration.strSandboxJailDir.length() == 0)
            Listener::configuration.strSandboxJailDir = "jail/";
        else if (Listener::configuration.strSandboxJailDir.at(Listener::configuration.strSandboxJailDir.length() - 1) != '/')
            Listener::configuration.strSandboxJailDir += "/";

        removeDirectoryContent(Listener::configuration.strSandboxJailDir);

        if (Listener::configuration.strSandboxScriptsDir.length() == 0)
        {
            if (Listener::configuration.bInFrameworkBuildDir)
                Listener::configuration.strSandboxScriptsDir = MASH_SOURCE_DIR "sandbox/";
            else
                Listener::configuration.strSandboxScriptsDir = Listener::configuration.strScriptsDir;
        }
        else if (Listener::configuration.strSandboxScriptsDir.at(Listener::configuration.strSandboxScriptsDir.length() - 1) != '/')
        {
            Listener::configuration.strSandboxScriptsDir += "/";
        }

        if (Listener::configuration.strSandboxTempDir.length() == 0)
            Listener::configuration.strSandboxTempDir = "./";
        else if (Listener::configuration.strSandboxTempDir.at(Listener::configuration.strSandboxTempDir.length() - 1) != '/')
            Listener::configuration.strSandboxTempDir += "/";

        removeDirectoryContent(Listener::configuration.strSandboxTempDir);

        if (Listener::configuration.strSourceHeuristics.length() == 0)
        {
            if (!Listener::configuration.bNoCompilation)
                Listener::configuration.strSourceHeuristics = Listener::configuration.strRepository;
#ifdef MASH_HEURISTIC_LOCATIONS
            else if (Listener::configuration.bInFrameworkBuildDir)
                Listener::configuration.strSourceHeuristics = MASH_HEURISTIC_LOCATIONS;
#endif
        }
    
        Listener::configuration.strSourceHeuristics = checkPaths(Listener::configuration.strSourceHeuristics);

#ifdef MASH_CLASSIFIER_LOCATIONS
        if (Listener::configuration.bInFrameworkBuildDir && (Listener::configuration.strSourceClassifiers.length() == 0))
            Listener::configuration.strSourceClassifiers = MASH_CLASSIFIER_LOCATIONS;
#endif

        Listener::configuration.strSourceClassifiers = checkPaths(Listener::configuration.strSourceClassifiers);

#ifdef MASH_GOALPLANNER_LOCATIONS
        if (Listener::configuration.bInFrameworkBuildDir && (Listener::configuration.strSourcePlanners.length() == 0))
            Listener::configuration.strSourcePlanners = MASH_GOALPLANNER_LOCATIONS;
#endif

        Listener::configuration.strSourcePlanners = checkPaths(Listener::configuration.strSourcePlanners);

#ifdef MASH_INSTRUMENT_LOCATIONS
        if (Listener::configuration.bInFrameworkBuildDir && (Listener::configuration.strSourceInstruments.length() == 0))
            Listener::configuration.strSourceInstruments = MASH_INSTRUMENT_LOCATIONS;
#endif

        Listener::configuration.strSourceInstruments = checkPaths(Listener::configuration.strSourceInstruments);
    }
}


ServerListener* Listener::createListener(int socket)
{
    return new Listener(socket);
}


ServerListener::tAction Listener::handleStatusCommand(const ArgumentsList& arguments)
{
    if (!sendResponse("READY", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleInfoCommand(const ArgumentsList& arguments)
{
    ArgumentsList responseArguments;
    
    responseArguments.add("ExperimentServer");
    if (!sendResponse("TYPE", responseArguments))
        return ACTION_CLOSE_CONNECTION;

    responseArguments.clear();
    responseArguments.add("1.6");
    if (!sendResponse("PROTOCOL", responseArguments))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleDoneCommand(const ArgumentsList& arguments)
{
    if (!_bReportsGenerated)
        generateReports();

    sendResponse("GOODBYE", ArgumentsList());
    return ACTION_CLOSE_CONNECTION;
}


ServerListener::tAction Listener::handleLogsCommand(const ArgumentsList& arguments)
{
    const int64_t MAX_SIZE = 200 * 1024;
    
    unsigned char* pBuffer = 0;
    int size = 0;
        
    size = _outStream.dump(&pBuffer, MAX_SIZE);
    if (size > 0)
    {
        ArgumentsList args;
        args.add("ExperimentServer.log");
        args.add(size);
        sendResponse("LOG_FILE", args);
        sendData((const unsigned char*) pBuffer, size);
        delete[] pBuffer;
    }

    size = _clientStream.dump(&pBuffer, MAX_SIZE);
    if (size > 0)
    {
        ArgumentsList args;
        args.add("ExperimentServerClient.log");
        args.add(size);
        sendResponse("LOG_FILE", args);
        sendData((const unsigned char*) pBuffer, size);
        delete[] pBuffer;
    }

    for (unsigned int i = 0; i < _taskController->getNbLogFiles(); ++i)
    {
        string strName;
        size = _taskController->getLogFileContent(i, strName, &pBuffer, MAX_SIZE);
        if (size > 0)
        {
            ArgumentsList args;
            args.add(strName + ".log");
            args.add(size);
            sendResponse("LOG_FILE", args);
            sendData((const unsigned char*) pBuffer, size);
            delete[] pBuffer;
        }
    }

    Client* pClient = _taskController->getClient();

    if (pClient && pClient->sendCommand("LOGS", ArgumentsList()))
    {
        string          strResponse;
        ArgumentsList   responseArgs;
    
        while (true)
        {
            if (!pClient->waitResponse(&strResponse, &responseArgs))
                break;

            if ((strResponse != "LOG_FILE") || (responseArgs.size() != 2))
                break;
            
            int size = responseArgs.getInt(1);
            unsigned char* pBuffer = new unsigned char[size];
        
            if (!pClient->waitData(pBuffer, size))
            {
                delete[] pBuffer;
                break;
            }
        
            ArgumentsList args;
            args.add(responseArgs.getString(0));
            args.add(size);
            sendResponse("LOG_FILE", args);
            sendData(pBuffer, size);

            delete[] pBuffer;
        }
    }
    
    if (!sendResponse("END_LOGS", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleSleepCommand(const ArgumentsList& arguments)
{
    sendResponse("OK", ArgumentsList());
    return ACTION_SLEEP;
}


ServerListener::tAction Listener::handleResetCommand(const ArgumentsList& arguments)
{
    if (_taskController && _taskController->getClient() &&
        _taskController->getClient()->sendCommand("RESET", ArgumentsList()))
    {
        string          strResponse;
        ArgumentsList   responseArgs;

        _taskController->getClient()->waitResponse(&strResponse, &responseArgs);
    }

    Client* pClient = (_taskController ? _taskController->getClient() : 0);
    bool bDetection = (dynamic_cast<ClassificationTask*>(_taskController) ?
                            dynamic_cast<ClassificationTask*>(_taskController)->isDoingDetection() :
                            false);

    delete _taskController;

    _bGlobalSeedSelected = false;
    _mode                = MODE_NORMAL;
    _bTrainingDone       = false;
    _error               = false;
    _bExperimentDone     = false;
    _bReportsGenerated   = false;

    if (_task == TASK_CLASSIFICATION)
    {
        _taskController = new ClassificationTask(this, bDetection);
    }
    else if (_task == TASK_GOALPLANNING)
    {
        _taskController = new GoalPlanningTask(this);
    }
    else
    {
        if (!sendResponse("ERROR", ArgumentsList("Nothing to reset")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Configure the task controller
    tError result = setupTaskController();
    if (result != ERROR_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList(getErrorDescription(result))))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Let the Task Controller know about the client to use
    if (pClient)
    {
        tError ret = _taskController->setClient(pClient);
        if (ret != ERROR_NONE)
        {
            delete _taskController;
            delete pClient;
            
            _taskController = 0;
            _task = TASK_NONE;

            if (!sendResponse("ERROR", ArgumentsList(getErrorDescription(ret))))
                return ACTION_CLOSE_CONNECTION;

            return ACTION_NONE;
        }
    }
    
    // Report stuff
    _xmlReportDocument = TiXmlDocument();

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	_xmlReportDocument.LinkEndChild(decl);
    
	TiXmlElement* xmlConfiguration = new TiXmlElement("configuration");
	_xmlReportDocument.LinkEndChild(xmlConfiguration);

	TiXmlElement* xmlExperiment = new TiXmlElement("experiment");

    if (_task == TASK_CLASSIFICATION)
    {
        if (bDetection)
            xmlExperiment->SetAttribute("type", "ObjectDetection");
        else
            xmlExperiment->SetAttribute("type", "Classification");
    }
    else
    {
        xmlExperiment->SetAttribute("type", "GoalPlanning");
    }
	xmlConfiguration->LinkEndChild(xmlExperiment);
    
    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleSetExperimentTypeCommand(const ArgumentsList& arguments)
{
    // Declarations
    string          strResponse;
    ArgumentsList   responseArgs;

    // Check the arguments
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we aren't connected to an application server yet
    if (_task != TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("Already connected to an application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Retrieve the type of experiment and check it
    string strExperimentType = arguments.getString(0);
    if (strExperimentType == "Classification")
    {
        _task = TASK_CLASSIFICATION;
        _taskController = new ClassificationTask(this, false);
    }
	else if (strExperimentType == "ObjectDetection")
    {
        _task = TASK_CLASSIFICATION;
        _taskController = new ClassificationTask(this, true);
		
    }
    else if (strExperimentType == "GoalPlanning")
    {
        _task = TASK_GOALPLANNING;
        _taskController = new GoalPlanningTask(this);
    }
    else
    {
        if (!sendResponse("UNKNOWN_TYPE", ArgumentsList(strExperimentType)))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Report stuff
	TiXmlElement* xmlReport = new TiXmlElement("configuration");
	_xmlReportDocument.LinkEndChild(xmlReport);

	TiXmlElement* xmlExperiment = new TiXmlElement("experiment");
    xmlExperiment->SetAttribute("type", strExperimentType);
	xmlReport->LinkEndChild(xmlExperiment);
 
    // Configure the task controller
    tError result = setupTaskController();
    if (result != ERROR_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList(getErrorDescription(result))))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleUseApplicationServerCommand(const ArgumentsList& arguments)
{
    // Declarations
    string          strResponse;
    ArgumentsList   responseArgs;

    // Check the arguments
    if (arguments.size() != 2)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we aren't connected to an application server yet
    if (_taskController->getClient())
    {
        if (!sendResponse("ERROR", ArgumentsList("Already connected to an application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Connect to the application server
    Client* pClient = new Client(&_clientStream);
    
    if (!pClient->connect(arguments.getString(0), arguments.getInt(1)))
    {
        delete pClient;
        
        if (!sendResponse("FAILED_TO_CONNECT", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    // Check that the application server isn't busy
    if (!pClient->sendCommand("STATUS", ArgumentsList()))
    {
        delete pClient;

        if (!sendResponse("ERROR", ArgumentsList("Failed to communicate with the application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    if (!pClient->waitResponse(&strResponse, &responseArgs))
    {
        delete pClient;
        
        if (!sendResponse("ERROR", ArgumentsList("Failed to communicate with the application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    if (strResponse != "READY")
    {
        delete pClient;
        
        if (!sendResponse("APPLICATION_SERVER_BUSY", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Let the Task Controller know about the client to use
    tError ret = _taskController->setClient(pClient);
    if (ret != ERROR_NONE)
    {
        delete _taskController;
        delete pClient;
            
        _taskController = 0;
        _task = TASK_NONE;

        if (!sendResponse("ERROR", ArgumentsList(getErrorDescription(ret))))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }


    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleUseGlobalSeedCommand(const ArgumentsList& arguments)
{
    // Check the arguments
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that the global seed wasn't already set
    if (_bGlobalSeedSelected)
    {
        if (!sendResponse("GLOBAL_SEED_ALREADY_SET", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are connected to an application server
    if (!_taskController->getClient())
    {
        if (!sendResponse("ERROR", ArgumentsList("Not connected to an application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Retrieve the global seed
    unsigned int seed = (unsigned int) arguments.getInt(0);

    // Provides it to the task controller
    _taskController->setGlobalSeed(seed);

    _bGlobalSeedSelected = true;

    // Report stuff
    TiXmlHandle docHandle(&_xmlReportDocument);
    TiXmlElement* xmlExperiment = docHandle.FirstChild("configuration").FirstChild("experiment").ToElement();
    if (xmlExperiment)
        xmlExperiment->SetAttribute("globalseed", seed);

    sendResponse("OK", ArgumentsList());

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleBeginExperimentSetupCommand(const ArgumentsList& arguments)
{
    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are connected to an application server
    if (!_taskController->getClient())
    {
        if (!sendResponse("ERROR", ArgumentsList("Not connected to an application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we aren't already doing the setup of the experiment
    if (_mode != MODE_NORMAL)
    {
        if (!sendResponse("ERROR", ArgumentsList("Experiment setup already started")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that the global seed was set
    if (!_bGlobalSeedSelected)
    {
        unsigned int seed = time(0);
        
        // Report stuff
        TiXmlHandle docHandle(&_xmlReportDocument);
        TiXmlElement* xmlExperiment = docHandle.FirstChild("configuration").FirstChild("experiment").ToElement();
        if (xmlExperiment)
            xmlExperiment->SetAttribute("globalseed", seed);

        _taskController->setGlobalSeed(seed);
        _bGlobalSeedSelected = true;
    }

    // Starts the setup
    _mode = MODE_EXPERIMENT_SETUP;
    _experimentParameters.clear();

    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleEndExperimentSetupCommand(const ArgumentsList& arguments)
{
    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are connected to an application server
    if (!_taskController->getClient())
    {
        if (!sendResponse("ERROR", ArgumentsList("Not connected to an application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are doing the setup of the experiment
    if (_mode != MODE_EXPERIMENT_SETUP)
    {
        if (!sendResponse("ERROR", ArgumentsList("Experiment setup not started")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    // Ends the setup
    _mode = MODE_NORMAL;

    ::TaskController::tResult ret = _taskController->setParameters(_experimentParameters);
    if (ret.error != ERROR_NONE)
    {
        ArgumentsList args;
        args.add(getErrorDescription(ret.error));

        if (!ret.strDetails.empty())
            args.add(ret.strDetails);

        if (!sendResponse("ERROR", args))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Report stuff
    TiXmlHandle docHandle(&_xmlReportDocument);
    TiXmlElement* xmlExperiment = docHandle.FirstChild("configuration").FirstChild("experiment").ToElement();
    if (xmlExperiment)
    {
        tExperimentParametersIterator iter, iterEnd;
        for (iter =  _experimentParameters.begin(), iterEnd = _experimentParameters.end();
             iter != iterEnd; ++iter)
        {
        	TiXmlElement* xmlSetting = new TiXmlElement("setting");
            xmlSetting->SetAttribute("name", iter->first);
            
            for (unsigned int i = 0; i < iter->second.size(); ++i)
            {
            	TiXmlElement* xmlArgument = new TiXmlElement("argument");
                xmlArgument->SetAttribute("value", iter->second.getString(i));
        	    xmlSetting->LinkEndChild(xmlArgument);
    	    }
    	    
    	    xmlExperiment->LinkEndChild(xmlSetting);
        }
    }

    _taskController->fillReport(configuration.strOutputDir);

    _experimentParameters.clear();

    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleUseInstrumentCommand(const ArgumentsList& arguments)
{
    // Check that an instrument name was provided
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Load the instrument
    ::TaskController::tResult res = _taskController->loadInstrument(arguments.getString(0));
    if (res.error != ERROR_NONE)
    {
        ArgumentsList args;
        args.add(getErrorDescription(res.error));
        
        if (!res.strDetails.empty())
            args.add(res.strDetails);

        if (!sendResponse("ERROR", args))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Report stuff
    TiXmlHandle docHandle(&_xmlReportDocument);
    TiXmlElement* xmlInstruments = docHandle.FirstChild("configuration").FirstChild("instruments").ToElement();
    if (!xmlInstruments)
    {
        xmlInstruments = new TiXmlElement("instruments");
        
        TiXmlElement* xmlReport = docHandle.FirstChild("configuration").ToElement();
        xmlReport->LinkEndChild(xmlInstruments);
    }
    
	TiXmlElement* xmlInstrument = new TiXmlElement("instrument");
    xmlInstrument->SetAttribute("name", arguments.getString(0));
    xmlInstruments->LinkEndChild(xmlInstrument);

    // Sends a response to the client
    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleBeginInstrumentSetupCommand(const ArgumentsList& arguments)
{
    // Check that an instrument name was provided
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we aren't already doing the setup of an instrument
    if (_mode != MODE_NORMAL)
    {
        if (!sendResponse("ERROR", ArgumentsList("Instrument setup already started")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Starts the setup
    _mode = MODE_INSTRUMENT_SETUP;
    _strCurrentInstrument = arguments.getString(0);
    _instrumentParameters.clear();

    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleEndInstrumentSetupCommand(const ArgumentsList& arguments)
{
    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are doing the setup of the instrument
    if (_mode != MODE_INSTRUMENT_SETUP)
    {
        if (!sendResponse("ERROR", ArgumentsList("Instrument setup not started")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Ends the setup
    _mode = MODE_NORMAL;

    _taskController->setInstrumentParameters(_strCurrentInstrument, _instrumentParameters);

    // Report stuff
    TiXmlHandle docHandle(&_xmlReportDocument);
    TiXmlElement* xmlInstruments = docHandle.FirstChild("configuration").FirstChild("instruments").ToElement();
    if (xmlInstruments)
    {
        TiXmlElement* xmlInstrument = 0;
        string strName;
		while (xmlInstrument = (TiXmlElement*) xmlInstruments->IterateChildren(xmlInstrument))
		{
            xmlInstrument->QueryStringAttribute("name", &strName);
		    if (strName == _strCurrentInstrument)
                break;
		}
                
        if (xmlInstrument)
        {
            tExperimentParametersIterator iter, iterEnd;
            for (iter =  _instrumentParameters.begin(), iterEnd = _instrumentParameters.end();
                 iter != iterEnd; ++iter)
            {
            	TiXmlElement* xmlSetting = new TiXmlElement("setting");
                xmlSetting->SetAttribute("name", iter->first);
            
                for (unsigned int i = 0; i < iter->second.size(); ++i)
                {
                	TiXmlElement* xmlArgument = new TiXmlElement("argument");
                    xmlArgument->SetAttribute("value", iter->second.getString(i));
            	    xmlSetting->LinkEndChild(xmlArgument);
        	    }
    	    
        	    xmlInstrument->LinkEndChild(xmlSetting);
            }
        }
    }
    
    _instrumentParameters.clear();

    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleUsePredictorModel(const ArgumentsList& arguments)
{
    // Check that some data size was provided
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    int size = arguments.getInt(0);
    if (size <= 0)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we aren't using a predictor yet
    if (_taskController->isPredictorLoaded())
    {
        if (!sendResponse("ERROR", ArgumentsList("Predictor already selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    // Save the model file
    DataWriter writer;
    
    if (!writer.open("input_predictor.model"))
    {
        if (!sendResponse("ERROR", ArgumentsList("Failed to create a file")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    const int MAX_DATA_SIZE = 10 * 1024;
    unsigned char buffer[MAX_DATA_SIZE];
    
    while (size > 0)
    {
        int nb = min(size, MAX_DATA_SIZE);
        
        if (!waitData(buffer, nb))
        {
            if (!sendResponse("ERROR", ArgumentsList("Failed to retrieve the data")))
                return ACTION_CLOSE_CONNECTION;

            return ACTION_NONE;
        }

        writer.write(buffer, nb);
        
        size -= nb;
    }
    
    writer.close();

    configuration.strPredictorModel = "input_predictor.model";

    _predictorModel.open(configuration.strPredictorModel);

    // Sends a response to the client
    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleUsePredictorInternalData(const ArgumentsList& arguments)
{
    // Check that some data size was provided
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    int size = arguments.getInt(0);
    if (size <= 0)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we aren't using a predictor yet
    if (_taskController->isPredictorLoaded())
    {
        if (!sendResponse("ERROR", ArgumentsList("Predictor already selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    // Retrieve the internal data file
    DataWriter writer;
    
    if (!writer.open("input_predictor.internal"))
    {
        if (!sendResponse("ERROR", ArgumentsList("Failed to create a file")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    const int MAX_DATA_SIZE = 10 * 1024;
    unsigned char buffer[MAX_DATA_SIZE];
    
    while (size > 0)
    {
        int nb = min(size, MAX_DATA_SIZE);
        
        if (!waitData(buffer, nb))
        {
            if (!sendResponse("ERROR", ArgumentsList("Failed to retrieve the data")))
                return ACTION_CLOSE_CONNECTION;

            return ACTION_NONE;
        }

        writer.write(buffer, nb);
        
        size -= nb;
    }
    
    writer.close();

    configuration.strPredictorData = "input_predictor.internal";

    // Sends a response to the client
    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleUsePredictorCommand(const ArgumentsList& arguments)
{
    // Check that a predictor name was provided
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we aren't using a predictor yet
    if (_taskController->isPredictorLoaded())
    {
        if (!sendResponse("ERROR", ArgumentsList("Predictor already selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    // Load the predictor
    ::TaskController::tResult res = _taskController->loadPredictor(arguments.getString(0),
                                                                   configuration.strPredictorModel,
                                                                   configuration.strPredictorData,
                                                                   _predictorModel.predictorSeed());
    if (res.error != ERROR_NONE)
    {
        ArgumentsList args;
        args.add(getErrorDescription(res.error));
        
        if (!res.strDetails.empty())
            args.add(res.strDetails);

        if (!sendResponse("ERROR", args))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Report stuff
	TiXmlElement* xmlPredictor = new TiXmlElement("predictor");
    xmlPredictor->SetAttribute("name", arguments.getString(0));

    TiXmlHandle docHandle(&_xmlReportDocument);
    TiXmlElement* xmlReport = docHandle.FirstChild("configuration").ToElement();
    xmlReport->LinkEndChild(xmlPredictor);

    // Sends a response to the client
    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleBeginPredictorSetupCommand(const ArgumentsList& arguments)
{
    // Check that a predictor was selected
    if ((_task == TASK_NONE) || !_taskController->isPredictorLoaded())
    {
        if (!sendResponse("ERROR", ArgumentsList("No predictor selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we aren't already doing the setup of the predictor
    if (_mode != MODE_NORMAL)
    {
        if (!sendResponse("ERROR", ArgumentsList("Predictor setup already started")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Starts the setup
    _mode = MODE_PREDICTOR_SETUP;
    _predictorParameters.clear();

    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleEndPredictorSetupCommand(const ArgumentsList& arguments)
{
    // Check that a predictor was selected
    if ((_task == TASK_NONE) || !_taskController->isPredictorLoaded())
    {
        if (!sendResponse("ERROR", ArgumentsList("No predictor selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are doing the setup of the predictor
    if (_mode != MODE_PREDICTOR_SETUP)
    {
        if (!sendResponse("ERROR", ArgumentsList("Predictor setup not started")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Ends the setup
    _mode = MODE_NORMAL;

    if (!_taskController->setupPredictor(_predictorParameters))
    {
        if (!sendResponse("ERROR", ArgumentsList("Failed to setup the predictor")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Report stuff
    TiXmlHandle docHandle(&_xmlReportDocument);
    TiXmlElement* xmlPredictor = docHandle.FirstChild("configuration").FirstChild("predictor").ToElement();
    if (xmlPredictor)
    {
        tExperimentParametersIterator iter, iterEnd;
        for (iter =  _predictorParameters.begin(), iterEnd = _predictorParameters.end();
             iter != iterEnd; ++iter)
        {
        	TiXmlElement* xmlSetting = new TiXmlElement("setting");
            xmlSetting->SetAttribute("name", iter->first);
            
            for (unsigned int i = 0; i < iter->second.size(); ++i)
            {
            	TiXmlElement* xmlArgument = new TiXmlElement("argument");
                xmlArgument->SetAttribute("value", iter->second.getString(i));
        	    xmlSetting->LinkEndChild(xmlArgument);
    	    }
    	    
    	    xmlPredictor->LinkEndChild(xmlSetting);
        }
    }
    
    _predictorParameters.clear();

    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleUseHeuristicsRepositoryCommand(const ArgumentsList& arguments)
{
    // Test if we must compile the heuristics ourself
    if (configuration.bNoCompilation)
    {
        if (!sendResponse("OK", ArgumentsList("Compilation of the heuristics disabled")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
            
    // Check that a repository URL was provided
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Clone the heuristics repository
    std::ostringstream str;
    str << configuration.strScriptsDir << "manage.py " << (configuration.verbosity > 0 ? "" : "--quiet ")
        << "--clone \"" << arguments.getString(0) << "\" " << configuration.strRepository;
    int ret = system(str.str().c_str());

    // Inform the client about the result
    if (ret == 0)
    {
        if (!sendResponse("OK", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;
    }
    else
    {
        if (!sendResponse("ERROR", ArgumentsList()))
            return ACTION_CLOSE_CONNECTION;
    }

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleUseHeuristicCommand(const ArgumentsList& arguments)
{
    // Check that a heuristic name was provided
    if (arguments.size() != 1)
    {
        if (!sendResponse("INVALID_ARGUMENTS", arguments))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are connected to an application server
    if (!_taskController->getClient())
    {
        if (!sendResponse("ERROR", ArgumentsList("Not connected to an application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    if (!configuration.bNoCompilation)
    {
        // Retrieve the path to the source file, and check that it exists
        string strSourceFile = configuration.strRepository + "/" + HeuristicsManager::getSourceFilePath(arguments.getString(0));
        struct stat fileInfo;
        if (stat(strSourceFile.c_str(), &fileInfo) != 0)
        {
            if (!sendResponse("ERROR", ArgumentsList("File not found: '" + strSourceFile + "'")))
                return ACTION_CLOSE_CONNECTION;

            return ACTION_NONE;
        }


        // Compile it
        char* cwd = getcwd(0, 0);

        std::ostringstream str;
        str << configuration.strScriptsDir << "manage.py " << (configuration.verbosity > 0 ? "" : "--quiet ")
            << "--compile \"" << strSourceFile << "\" \"" << arguments.getString(0) << "\" \""
            << configuration.strHeuristicsDir << "\" \"" << configuration.strBuildDir << "\" \"" << cwd << "\"";
        
        free(cwd);
        
        int ret = system(str.str().c_str());
        if (ret != 0)
        {
            if (!sendResponse("ERROR", ArgumentsList("Failed to compile the file '" + strSourceFile + "'")))
                return ACTION_CLOSE_CONNECTION;

            return ACTION_NONE;
        }
    }

    // Add it to the task-specific object
    FeaturesComputer* pFeaturesComputer = _taskController->getFeaturesComputer();
    if (!pFeaturesComputer->addHeuristic(arguments.getString(0), _predictorModel.heuristicSeed(arguments.getString(0))))
    {
        if (!sendResponse("ERROR", ArgumentsList("Failed to load the heuristic '" + arguments.getString(0) + "'")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Report stuff
    TiXmlHandle docHandle(&_xmlReportDocument);
    TiXmlElement* xmlHeuristics = docHandle.FirstChild("configuration").FirstChild("heuristics").ToElement();
    if (!xmlHeuristics)
    {
        xmlHeuristics = new TiXmlElement("heuristics");
        
        TiXmlElement* xmlReport = docHandle.FirstChild("configuration").ToElement();
        xmlReport->LinkEndChild(xmlHeuristics);
    }
    
	TiXmlElement* xmlHeuristic = new TiXmlElement("heuristic");
    xmlHeuristic->SetAttribute("index", _taskController->getFeaturesComputer()->nbHeuristics() - 1);
    xmlHeuristic->SetAttribute("name", arguments.getString(0));
    xmlHeuristics->LinkEndChild(xmlHeuristic);

    // Sends a response to the client
    if (!sendResponse("OK", ArgumentsList()))
        return ACTION_CLOSE_CONNECTION;

    return ACTION_NONE;
}


ServerListener::tAction Listener::handleTrainPredictorCommand(const ArgumentsList& arguments)
{
    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("Not doing an experiment")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are connected to an application server
    if (!_taskController->getClient())
    {
        if (!sendResponse("ERROR", ArgumentsList("Not connected to an application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that a predictor was selected
    if (!_taskController->isPredictorLoaded())
    {
        if (!sendResponse("ERROR", ArgumentsList("No predictor selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }
    
    // Save the configuration report
    _xmlReportDocument.SaveFile(configuration.strOutputDir + "configuration.xml");
    _xmlReportDocument = TiXmlDocument();

    // Train the predictor
    _bTrainingDone = true;
    
    return _taskController->train();
}


ServerListener::tAction Listener::handleTestPredictorCommand(const ArgumentsList& arguments)
{
    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("Not doing an experiment")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that we are connected to an application server
    if (!_taskController->getClient())
    {
        if (!sendResponse("ERROR", ArgumentsList("Not connected to an application server")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Check that a predictor was selected
    if (!_taskController->isPredictorLoaded())
    {
        if (!sendResponse("ERROR", ArgumentsList("No predictor selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Classification
    return _taskController->test();
}


ServerListener::tAction Listener::handleReportDataCommand(const ArgumentsList& arguments)
{
    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    // Notify the instruments about the end of the experiment (if necessary)
    if (!_bExperimentDone)
    {
        if ((configuration.sandboxingMechanisms & SANDBOXING_INSTRUMENTS) &&
            !_taskController->getInstrumentsSet()->onExperimentDone())
        {
            if (!sendResponse("ERROR", ArgumentsList("Failed to close the reports")))
                return ACTION_CLOSE_CONNECTION;

            return ACTION_NONE;
        }
    }
    
    _bExperimentDone = true;

    // Generate all the missing reports
    if (!_bReportsGenerated)
        generateReports();

    // Compress all the reports
    char* current_directory = getcwd(0, 0);
    chdir(configuration.strOutputDir.c_str());
    
    string command = "tar -czf reports.tar.gz --exclude reports.tar.gz ";

    if (!_bTrainingDone)
        command += "--exclude predictor.model --exclude predictor.internal ";
    
    command += "*";
    
    system(command.c_str());
    chdir(current_directory);
    free(current_directory);

    // Sends the reports to the client
    FILE* pFile = fopen((configuration.strOutputDir + "reports.tar.gz").c_str(), "rb");
    fseek(pFile, 0, SEEK_END);
    size_t size = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    bool result = sendResponse("DATA", ArgumentsList((int) size));
    
    const unsigned int MAX_SIZE = 1024 * 1024;
    unsigned char buffer[MAX_SIZE];
    size_t counter = 0;
    
    while (result && (counter < size))
    {
        size_t nb = fread(buffer, sizeof(unsigned char), MAX_SIZE, pFile);
        if (nb <= 0)
        {
            result = false;
            break;
        }

        result = sendData(buffer, nb);

        counter += nb;
    }
    
    fclose(pFile);

    return (result ? ACTION_NONE : ACTION_CLOSE_CONNECTION);
}


ServerListener::tAction Listener::handleReportErrorsCommand(const ArgumentsList& arguments)
{
    // Check that an experiment type was selected
    if (_task == TASK_NONE)
    {
        if (!sendResponse("ERROR", ArgumentsList("No experiment type selected")))
            return ACTION_CLOSE_CONNECTION;

        return ACTION_NONE;
    }

    return _taskController->reportErrors();
}


tError Listener::setupTaskController()
{
    // Assertions
    assert(_taskController);

    // Configure the task controller. For simplicity, the configurations of the
    // sandboxes are all filled even if not used
    tTaskControllerConfiguration cfg;
    tSandboxConfiguration        predictorSandboxConfiguration;
    tSandboxConfiguration        heuristicsSandboxConfiguration;
    tSandboxConfiguration        instrumentsSandboxConfiguration;

    // Common fields
    predictorSandboxConfiguration.verbosity             = configuration.verbosity;
    predictorSandboxConfiguration.strUsername           = configuration.strSandboxUsername;
    predictorSandboxConfiguration.strLogDir             = Server::strLogFolder;
    predictorSandboxConfiguration.strOutputDir          = configuration.strOutputDir;
    predictorSandboxConfiguration.strScriptsDir         = configuration.strSandboxScriptsDir;
    predictorSandboxConfiguration.strTempDir            = configuration.strSandboxTempDir;
    predictorSandboxConfiguration.bDeleteAllLogFiles    = !configuration.bStandalone;

    if (!configuration.strCoreDumpTemplate.empty())
        predictorSandboxConfiguration.strCoreDumpTemplate = configuration.strCoreDumpTemplate;

    heuristicsSandboxConfiguration  = predictorSandboxConfiguration;
    instrumentsSandboxConfiguration = predictorSandboxConfiguration;

    // Predictor-specific
    predictorSandboxConfiguration.strJailDir    = configuration.strSandboxJailDir + "predictor/";
    predictorSandboxConfiguration.strSourceDir  = (_task == TASK_CLASSIFICATION ?
                                                    configuration.strSourceClassifiers :
                                                    configuration.strSourcePlanners);

    // Heuristics-specific
    heuristicsSandboxConfiguration.strJailDir   = configuration.strSandboxJailDir + "heuristics/";
    heuristicsSandboxConfiguration.strSourceDir = configuration.strSourceHeuristics;

    // Instruments-specific
    instrumentsSandboxConfiguration.strJailDir   = configuration.strSandboxJailDir + "instruments/";
    instrumentsSandboxConfiguration.strSourceDir = configuration.strSourceInstruments;

    // Task controller configuration
    cfg.strPredictorsFolder     = (_task == TASK_CLASSIFICATION ?
                                        configuration.strClassifiersDir :
                                        configuration.strPlannersDir);
    cfg.strHeuristicsFolder     = configuration.strHeuristicsDir;
    cfg.strInstrumentsFolder    = configuration.strInstrumentsDir;
    cfg.strLogFolder            = Server::strLogFolder;
    cfg.strReportFolder         = configuration.strOutputDir;
    cfg.strCaptureFolder        = ((_task == TASK_GOALPLANNING) && configuration.bStandalone ? 
                                        configuration.strCaptureDir : "");

    cfg.predictorSandboxConfiguration   = (configuration.sandboxingMechanisms & SANDBOXING_PREDICTOR ?
                                                &predictorSandboxConfiguration : 0);
    cfg.heuristicsSandboxConfiguration  = (configuration.sandboxingMechanisms & SANDBOXING_HEURISTICS ?
                                                &heuristicsSandboxConfiguration : 0);
    cfg.instrumentsSandboxConfiguration = (configuration.sandboxingMechanisms & SANDBOXING_INSTRUMENTS ?
                                                &instrumentsSandboxConfiguration : 0);

    // The actual setup
    return _taskController->setup(cfg);
}


void Listener::generateReports()
{
    DataWriter writer;

    SandboxedHeuristicsSet* pHeuristicsSet = dynamic_cast<SandboxedHeuristicsSet*>(
                                _taskController->getFeaturesComputer()->heuristicsSet());

    if (pHeuristicsSet && writer.open(configuration.strOutputDir + "mash/heuristic_statistics.data"))
    {
        for (unsigned int i = 0; i < pHeuristicsSet->nbHeuristics(); ++i)
        {
            tHeuristicStatistics statistics;
    
            if (pHeuristicsSet->reportStatistics(i, &statistics))
            {
                writer << "HEURISTIC " << i << endl
                       << "TOTAL_DURATION " << statistics.total_duration << endl
                       << "INITIALIZATION_DURATION " << statistics.initialization.total_duration << endl
                       << "SEQUENCES_COUNT " << statistics.sequences.nb_events << endl
                       << "SEQUENCES_TOTAL_DURATION " << statistics.sequences.total_duration << endl
                       << "SEQUENCES_MEAN_DURATION " << statistics.sequences.mean_duration << endl
                       << "IMAGES_COUNT " << statistics.images.nb_events << endl
                       << "IMAGES_TOTAL_DURATION " << statistics.images.total_duration << endl
                       << "IMAGES_MEAN_DURATION " << statistics.images.mean_event_duration << endl
                       << "IMAGES_PIXELS_COUNT " << statistics.images.nb_subevents << endl
                       << "IMAGES_PIXELS_MEAN_DURATION " << statistics.images.mean_subevent_duration << endl
                       << "POSITIONS_COUNT " << statistics.positions.nb_events << endl
                       << "POSITIONS_TOTAL_DURATION " << statistics.positions.total_duration << endl
                       << "POSITIONS_MEAN_DURATION " << statistics.positions.mean_event_duration << endl
                       << "POSITIONS_PIXELS_COUNT " << statistics.positions.nb_subevents << endl
                       << "POSITIONS_PIXELS_MEAN_DURATION " << statistics.positions.mean_subevent_duration << endl
                       << "FEATURES_COUNT " << statistics.features.nb_events << endl
                       << "FEATURES_TOTAL_DURATION " << statistics.features.total_duration << endl
                       << "FEATURES_MEAN_DURATION " << statistics.features.mean_duration << endl;
            }
        }
    }
    
    
    // Ask the predictor to save its model if necessary
    if (_bTrainingDone)
        _taskController->savePredictorModel();
    
    _bReportsGenerated = true;
}
