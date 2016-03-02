#include "listener.h"
#include <mash-network/server.h>
#include <mash-network/commands_serializer.h>
#include <mash-network/networkutils.h>
#include <mash-utils/stringutils.h>
#include <SimpleOpt.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <iostream>

#if USE_CURL
    #include <mash/curl_image_downloader.h>
#endif


using namespace Mash;
using namespace std;


/**************************** COMMAND-LINE PARSING ****************************/

enum tOptions
{
    // General
    OPT_HELP,
    OPT_HOST,
    OPT_PORT,
    OPT_EXPERIMENT_SETTINGS,
    OPT_SCRIPTS_DIR,
    OPT_LOG_FOLDER,
    OPT_OUTPUT_DIR,
    OPT_VERBOSE,
    OPT_VERBOSE1,
    OPT_VERBOSE2,
    OPT_VERBOSE3,
    OPT_VERBOSE4,
    OPT_VERBOSE5,

    // Goal-planning
    OPT_NB_ROUNDS,
    OPT_CAPTURE_DIR,

    // Heuristics
    OPT_NO_COMPILATION,
    OPT_REPORT_STATISTICS,
    OPT_REPOSITORY,
    OPT_HEURISTICS_DIR,
    OPT_BUILD_DIR,

    // Predictors
    OPT_CLASSIFIERS_DIR,
    OPT_PLANNERS_DIR,
    OPT_PREDICTOR_MODEL,
    OPT_PREDICTOR_DATA,
    OPT_PREDICTOR_NO_TRAINING,

    // Instruments
    OPT_INSTRUMENTS_DIR,

    // Sandboxing
    OPT_NO_SANDBOXING,
    OPT_NO_HEURISTICS_SANDBOXING,
    OPT_NO_PREDICTOR_SANDBOXING,
    OPT_NO_INSTRUMENTS_SANDBOXING,
    OPT_CORE_DUMP_TEMPLATE,
    OPT_SANDBOX_USERNAME,
    OPT_SANDBOX_JAIL_DIR,
    OPT_SANDBOX_SCRIPTS_DIR,
    OPT_SANDBOX_TEMP_DIR,
    OPT_SANDBOX_SOURCE_HEURISTICS,
    OPT_SANDBOX_SOURCE_CLASSIFIERS,
    OPT_SANDBOX_SOURCE_GOALPLANNERS,
    OPT_SANDBOX_SOURCE_INSTRUMENTS,
};


CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] =
{
    // General
    { OPT_HELP,                     "--help",           SO_NONE    },
    { OPT_HELP,                     "-h",               SO_NONE    },
    { OPT_HOST,                     "--host",           SO_REQ_CMB },
    { OPT_PORT,                     "--port",           SO_REQ_CMB },
    { OPT_EXPERIMENT_SETTINGS,      "--settings",       SO_REQ_CMB },
    { OPT_SCRIPTS_DIR,              "--scriptsdir",     SO_REQ_CMB },
    { OPT_LOG_FOLDER,               "--logfolder",      SO_REQ_CMB },
    { OPT_OUTPUT_DIR,               "--outputfolder",   SO_REQ_CMB },
    { OPT_VERBOSE,                  "--verbose",        SO_NONE    },
    { OPT_VERBOSE1,                 "-v",               SO_NONE    },
    { OPT_VERBOSE2,                 "-vv",              SO_NONE    },
    { OPT_VERBOSE3,                 "-vvv",             SO_NONE    },
    { OPT_VERBOSE4,                 "-vvvv",            SO_NONE    },
    { OPT_VERBOSE5,                 "-vvvvv",           SO_NONE    },

    // Goal-planning
    { OPT_NB_ROUNDS,                "--rounds",         SO_REQ_CMB },
    { OPT_CAPTURE_DIR,              "--capturefolder",  SO_REQ_CMB },

    // Heuristics
    { OPT_NO_COMPILATION,           "--no-compilation", SO_NONE    },
    { OPT_REPORT_STATISTICS,        "--statistics",     SO_NONE    },
    { OPT_REPOSITORY,               "--repository",     SO_REQ_CMB },
    { OPT_HEURISTICS_DIR,           "--heuristicsdir",  SO_REQ_CMB },
    { OPT_BUILD_DIR,                "--builddir",       SO_REQ_CMB },

    // Predictors
    { OPT_CLASSIFIERS_DIR,          "--classifiersdir",         SO_REQ_CMB },
    { OPT_PLANNERS_DIR,             "--plannersdir",            SO_REQ_CMB },
    { OPT_PREDICTOR_MODEL,          "--predictor-model",        SO_REQ_CMB },
    { OPT_PREDICTOR_DATA,           "--predictor-data",         SO_REQ_CMB },
    { OPT_PREDICTOR_NO_TRAINING,    "--predictor-no-training",  SO_NONE },

    // Instruments
    { OPT_INSTRUMENTS_DIR,          "--instrumentsdir", SO_REQ_CMB },

    // Sandboxing
    { OPT_NO_SANDBOXING,                "--no-sandboxing",              SO_NONE },
    { OPT_NO_HEURISTICS_SANDBOXING,     "--no-heuristics-sandboxing",   SO_NONE },
    { OPT_NO_PREDICTOR_SANDBOXING,      "--no-predictor-sandboxing",    SO_NONE },
    { OPT_NO_INSTRUMENTS_SANDBOXING,    "--no-instruments-sandboxing",  SO_NONE },
    { OPT_NO_SANDBOXING,                "--no-sandboxing",              SO_NONE },
    { OPT_CORE_DUMP_TEMPLATE,           "--coredump-template",          SO_REQ_CMB },
    { OPT_SANDBOX_USERNAME,             "--sandbox-username",           SO_REQ_CMB },
    { OPT_SANDBOX_JAIL_DIR,             "--sandbox-jaildir",            SO_REQ_CMB },
    { OPT_SANDBOX_SCRIPTS_DIR,          "--sandbox-scriptsdir",         SO_REQ_CMB },
    { OPT_SANDBOX_TEMP_DIR,             "--sandbox-tempdir",            SO_REQ_CMB },
    { OPT_SANDBOX_SOURCE_HEURISTICS,    "--source-heuristics",          SO_REQ_CMB },
    { OPT_SANDBOX_SOURCE_CLASSIFIERS,   "--source-classifiers",         SO_REQ_CMB },
    { OPT_SANDBOX_SOURCE_GOALPLANNERS,  "--source-goalplanners",        SO_REQ_CMB },
    { OPT_SANDBOX_SOURCE_INSTRUMENTS,   "--source-instruments",         SO_REQ_CMB },

    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "MASH Experiment Server" << endl
         << "Usage: " << strApplicationName << " [options]" << endl
         << endl
         << "This program can work in two modes:" << endl
         << "  - as a server executing the instructions sent by a client application" << endl
         << "    (the default)" << endl
         << "  - as a standalone program executing the instructions found in a text file" << endl
         << "    (if the --settings option is used)" << endl
         << endl
         << "IMPORTANT: the Experiments Server only serves one client at a time!" << endl
         << endl
         << "Options:" << endl
         << "    --help, -h:              Display this help" << endl
         << "    --host=<host>:           The host name or IP address that the server must listen on." << endl
         << "                             If not specified, the first available is used." << endl
         << "    --port=<post>:           The port that the server must listen on (default: 10000)" << endl
         << "    --settings=<FILE>:       Use the instructions found in FILE instead of" << endl
         << "                             those from a client (standalone mode)" << endl
         << "    --scriptsdir=<DIR>:      Path to the directory where the scripts used by the" << endl
         << "                             experiments server ('manage.py' and the 'heuristics_cmake'" << endl
         << "                             folder) are located (default: When executed from the build" << endl
         << "                             directory of the Framework, the correct path. Otherwise," << endl
         << "                             the current one)" << endl
         << "    --logfolder=<DIR>:       Path to the location of the log files (default: 'logs/')" << endl
         << "    --outputfolder=<DIR>:    Path where the data files written by the instruments must be" << endl
         << "                             saved (default: 'out/')" << endl
         << "    --verbose:" << endl      
         << "    -v, -vv, -vvv. -vvvv, -vvvvv:" << endl
         << "                             Verbose output (the more v's, the more verbose we are)"
         << endl
         << "Goal-planning-related options:" << endl
         << "    --rounds=<COUNT>:        Number of tests to perform (default: 1)" << endl
         << "    --capturefolder=<DIR>:   (standalone mode only) Path to the location where the images" << endl
         << "                             received from the Application Server are saved" << endl
         << endl
         << "Heuristics-related options:" << endl
         << "    --no-compilation:        Disable the compilation of the heuristics (assume" << endl
         << "                             that they are already compiled in --heuristicsdir)" << endl
         << "    --repository=<PATH>:     Path to the destination where the GIT repository of heuristics" << endl
         << "                             will be cloned (default: 'heuristics.git')" << endl
         << "    --heuristicsdir=<DIR>:   Path to the directory where the compiled heuristics" << endl
         << "                             must be put (default: 'heuristics')" << endl
         << "    --builddir=<DIR>:        Path to the directory to use to compile the heuristics" << endl
         << "                             (default: 'build')" << endl
         << endl
         << "Predictors-related options:" << endl
         << "    --classifiersdir=<DIR>:  Path to the directory where the classifiers are" << endl
         << "                             located (default: 'classifiers')" << endl
         << "    --goalplannersdir=<DIR>: Path to the directory where the goal-planners are" << endl
         << "                             located (default: 'goalplanners')" << endl
         << "    --predictor-model=<PATH>:" << endl
         << "                             Path to the model that the predictor must load" << endl
         << "                             (default: none)" << endl
         << "    --predictor-data=<PATH>: Path to the internal data to provide to the predictor" << endl
         << "                             alongside the model to load (default: none)" << endl
         << "    --predictor-no-training: When a model is loaded, disable the training phase" << endl
         << endl
         << "Instruments-related options:" << endl
         << "    --instrumentsdir=<DIR>:  Path to the directory where the instruments are" << endl
         << "                             located (default: 'instruments')" << endl
         << endl
         << "Sandboxing-related options:" << endl
         << "    --no-heuristics-sandboxing:" << endl
         << "                             Disable the sandboxing mechanism for the heuristics" << endl
         << "    --no-predictor-sandboxing:" << endl
         << "                             Disable the sandboxing mechanism for the predictor" << endl
         << "    --no-instruments-sandboxing:" << endl
         << "                             Disable the sandboxing mechanism for the instruments" << endl
         << "    --no-sandboxing:         Shortcut for '--no-heuristics-sandboxing --no-predictor-sandboxing" << endl
         << "                             --no-instruments-sandboxing'" << endl
         << "    --coredump-template=<TEMPLATE>:" << endl
         << "                             Template of the name of the core dump files (default: the" << endl
         << "                             value of the ${MASH_CORE_DUMP_TEMPLATE} compilation setting)" << endl
         << "    --sandbox-username=<NAME>:" << endl
         << "                             The untrusted plugins in the sandboxes will be run by this user." << endl
         << "                             Mandatory if the sandbox is run as root." << endl
         << "    --sandbox-jaildir=<DIR>: Path to the directory in which the folders that will serve" << endl
         << "                             as the root ('/') of the filesystem for the sandboxes will be" << endl
         << "                             created (default: 'jail')" << endl
         << "    --sandbox-scriptsdir=<DIR>:" << endl
         << "                             Path to the directory where the scripts used for the" << endl
         << "                             sandboxing ('coredump_analyzer.py' and the 'get_stack*.cmd'" << endl
         << "                             files) are located (default: When executed from the build" << endl
         << "                             directory of the Framework, the correct path. Otherwise, the" << endl
         << "                             same than --scriptsdir)" << endl
         << "    --sandbox-tempdir=<DIR>: Path to the directory to use to write temporary files" << endl
         << "                             during core dump analysis (default: the current one)" << endl
         << "    --source-heuristics=<DIR>:" << endl
         << "                             Paths to the directories (separated by ;) where the source code" << endl
         << "                             files of the heuristics are located (default: When --no-compilation" << endl
         << "                             isn't used, the same than --repository. When executed from the" << endl
         << "                             build directory of the Framework, the value of the" << endl
         << "                             ${MASH_HEURISTIC_LOCATIONS} compilation setting. Otherwise, none)." << endl
         << "    --source-classifiers=<DIR>:" << endl
         << "                             Path to the directory where the source code files of the" << endl
         << "                             classifiers are located (default: When executed from the" << endl
         << "                             build directory of the Framework, the value of the" << endl
         << "                             ${MASH_CLASSIFIER_LOCATIONS} compilation setting. Otherwise, none)." << endl
         << "    --source-goalplanners=<DIR>:" << endl
         << "                             Path to the directory where the source code files of the" << endl
         << "                             goal-planners are located (default: When executed from the" << endl
         << "                             build directory of the Framework, the value of the" << endl
         << "                             ${MASH_PLANNER_LOCATIONS} compilation setting. Otherwise, none)." << endl
         << "    --source-instruments=<DIR>:" << endl
         << "                             Path to the directory where the source code files of the" << endl
         << "                             instruments are located (default: When executed from the" << endl
         << "                             build directory of the Framework, the value of the" << endl
         << "                             ${MASH_INSTRUMENT_LOCATIONS} compilation setting. Otherwise, none)." << endl;
}


int main(int argc, char** argv)
{
    // Declarations
    tListenerConfiguration  configuration;
    string                  strExperimentSettings = "";
    unsigned int            nbRounds              = 1;
    bool                    bNoTraining           = false;

    // Parse the command-line arguments
    CSimpleOpt args(argc, argv, COMMAND_LINE_OPTIONS);
    while (args.Next())
    {
        if (args.LastError() == SO_SUCCESS)
        {
            switch (args.OptionId())
            {
                //_____ General _____
                
                case OPT_HELP:
                    showUsage(argv[0]);
                    return 0;
                
                case OPT_HOST:
                    configuration.strHost = args.OptionArg();
                    break;

                case OPT_PORT:
                    configuration.port = StringUtils::parseUnsignedInt(args.OptionArg());
                    break;

                case OPT_EXPERIMENT_SETTINGS:
                    configuration.bStandalone = true;
                    strExperimentSettings = args.OptionArg();
                    break;

                case OPT_SCRIPTS_DIR:
                    configuration.strScriptsDir = args.OptionArg();
                    break;

                case OPT_LOG_FOLDER:
                    Server::strLogFolder = args.OptionArg();
                    if (Server::strLogFolder[Server::strLogFolder.size() - 1] != '/')
                        Server::strLogFolder += "/";
                    break;

                case OPT_OUTPUT_DIR:
                    configuration.strOutputDir = args.OptionArg();
                    break;

                case OPT_VERBOSE:
                    configuration.verbosity = max(configuration.verbosity, (unsigned int) 1);
                    break;

                case OPT_VERBOSE1:
                case OPT_VERBOSE2:
                case OPT_VERBOSE3:
                case OPT_VERBOSE4:
                case OPT_VERBOSE5:
                    configuration.verbosity = max(configuration.verbosity, (unsigned int) (args.OptionId() - OPT_VERBOSE));
                    break;


                //_____ Goal-planning ______

                case OPT_NB_ROUNDS:
                    nbRounds = StringUtils::parseUnsignedInt(args.OptionArg());
                    break;

                case OPT_CAPTURE_DIR:
                    configuration.strCaptureDir = args.OptionArg();
                    break;


                //_____ Heuristics _____

                case OPT_NO_COMPILATION:
                    configuration.bNoCompilation = true;
                    break;

                case OPT_REPOSITORY:
                    configuration.strRepository = args.OptionArg();
                    break;

                case OPT_HEURISTICS_DIR:
                    configuration.strHeuristicsDir = args.OptionArg();
                    break;

                case OPT_BUILD_DIR:
                    configuration.strBuildDir = args.OptionArg();
                    break;


                //_____ Predictors ______

                case OPT_CLASSIFIERS_DIR:
                    configuration.strClassifiersDir = args.OptionArg();
                    break;

                case OPT_PLANNERS_DIR:
                    configuration.strPlannersDir = args.OptionArg();
                    break;

                case OPT_PREDICTOR_MODEL:
                    configuration.strPredictorModel = args.OptionArg();
                    break;

                case OPT_PREDICTOR_DATA:
                    configuration.strPredictorData = args.OptionArg();
                    break;
                    
                case OPT_PREDICTOR_NO_TRAINING:
                    bNoTraining = true;
                    break;


                //_____ Instruments ______

                case OPT_INSTRUMENTS_DIR:
                    configuration.strInstrumentsDir = args.OptionArg();
                    break;

            
                //_____ Sandboxing _____

                case OPT_NO_SANDBOXING:
                    configuration.sandboxingMechanisms = 0;
                    break;

                case OPT_NO_HEURISTICS_SANDBOXING:
                    configuration.sandboxingMechanisms &= ~SANDBOXING_HEURISTICS;
                    break;

                case OPT_NO_PREDICTOR_SANDBOXING:
                    configuration.sandboxingMechanisms &= ~SANDBOXING_PREDICTOR;
                    break;

                case OPT_NO_INSTRUMENTS_SANDBOXING:
                    configuration.sandboxingMechanisms &= ~SANDBOXING_INSTRUMENTS;
                    break;

                case OPT_CORE_DUMP_TEMPLATE:
                    configuration.strCoreDumpTemplate = args.OptionArg();
                    break;

                case OPT_SANDBOX_USERNAME:
                    configuration.strSandboxUsername = args.OptionArg();
                    break;

                case OPT_SANDBOX_JAIL_DIR:
                    configuration.strSandboxJailDir = args.OptionArg();
                    break;

                case OPT_SANDBOX_SCRIPTS_DIR:
                    configuration.strSandboxScriptsDir = args.OptionArg();
                    break;

                case OPT_SANDBOX_TEMP_DIR:
                    configuration.strSandboxTempDir = args.OptionArg();
                    break;

                case OPT_SANDBOX_SOURCE_HEURISTICS:
                    configuration.strSourceHeuristics = args.OptionArg();
                    break;

                case OPT_SANDBOX_SOURCE_CLASSIFIERS:
                    configuration.strSourceClassifiers = args.OptionArg();
                    break;

                case OPT_SANDBOX_SOURCE_GOALPLANNERS:
                    configuration.strSourcePlanners = args.OptionArg();
                    break;

                case OPT_SANDBOX_SOURCE_INSTRUMENTS:
                    configuration.strSourceInstruments = args.OptionArg();
                    break;
            }
        }
        else
        {
            cerr << "Invalid argument: " << args.OptionText() << endl;
            return -1;
        }
    }

    // Determine if we are running from the build directory
    struct stat fileInfo;
    configuration.bInFrameworkBuildDir = (stat("MASH_FRAMEWORK", &fileInfo) == 0);


#if USE_CURL
    ImageUtils::setDownloader(new CURLImageDownloader());
#endif


    // Initialize the listener
    Listener::initialize(configuration);
    
    OutStream::verbosityLevel = configuration.verbosity;
    
    if (!configuration.bStandalone)
    {
        cout << "********************************************************************************" << endl
             << "* Experiment Server - server mode" << endl
             << "********************************************************************************" << endl
             << endl;

        // Start handling requests from clients
        Server server(1, 100, "ExperimentServer");
        server.listen(configuration.strHost, configuration.port, Listener::createListener);
    }
    else
    {
        cout << "********************************************************************************" << endl
             << "* Experiment Server - standalone mode" << endl
             << "********************************************************************************" << endl
             << endl;

        // Read the file containing the settings
        CommandsSerializer serializer;
        if (!serializer.deserialize(strExperimentSettings))
        {
            cerr << "Failed to process the file '" << strExperimentSettings << "'" << endl;
            return -1;
        }
        
        // Creates a listener, we'll pretend to be a server
        Listener* pListener = (Listener*) Listener::createListener(0);
        
        // Setup the experiment according to the settings file
        for (unsigned int i = 0; i < serializer.nbCommands(); ++i)
        {
            CommandsSerializer::tCommand command = serializer.getCommand(i);
            
            ServerListener::tAction action = pListener->handleCommand(command.strCommand, command.arguments);
            
            if ((action != ServerListener::ACTION_NONE) || pListener->failure())
            {
                pListener->handleCommand("REPORT_ERRORS", ArgumentsList());
                delete pListener;
                return -1;
            }
        }
        
        if (pListener->task() == Listener::TASK_NONE)
        {
            delete pListener;
            return -1;
        }
        
        // Training
        if (!bNoTraining || configuration.strPredictorModel.empty())
        {
            pListener->handleCommand("TRAIN_PREDICTOR", ArgumentsList());

            if (pListener->failure())
            {
                pListener->handleCommand("REPORT_ERRORS", ArgumentsList());
                delete pListener;
                return -1;
            }
        }

        // Test
        for (unsigned int i= 0; i < (pListener->task() == Listener::TASK_GOALPLANNING ? nbRounds : 1); ++i)
        {
            pListener->handleCommand("TEST_PREDICTOR", ArgumentsList());

            if (pListener->failure())
            {
                pListener->handleCommand("REPORT_ERRORS", ArgumentsList());
                delete pListener;
                return -1;
            }
        }

        pListener->handleCommand("DONE", ArgumentsList());

        delete pListener;
    }
    
    return 0;
}
