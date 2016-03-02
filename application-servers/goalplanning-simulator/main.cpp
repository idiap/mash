#include "goalplanning_simulator.h"
#include <mash-appserver/interactive_application_server.h>
#include <mash-utils/stringutils.h>
#include <mash-network/commands_serializer.h>
#include <SimpleOpt.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <iostream>

using namespace Mash;
using namespace std;


/**************************** COMMAND-LINE PARSING ****************************/

enum tOptions
{
    OPT_HOST,
    OPT_PORT,
    OPT_CONFIG,
    OPT_LOG_FOLDER,
    OPT_VERBOSE,
    OPT_HELP,
};

CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] =
{
    { OPT_HOST,         "--host",       SO_REQ_CMB },
    { OPT_PORT,         "--port",       SO_REQ_CMB },
    { OPT_CONFIG,       "--config",     SO_REQ_CMB },
    { OPT_LOG_FOLDER,   "--logfolder",  SO_REQ_CMB },
    { OPT_VERBOSE,      "--verbose",    SO_NONE    },
    { OPT_HELP,         "--help",       SO_NONE    },
    { OPT_HELP,         "-h",           SO_NONE    },
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "MASH Goal-planning Simulator" << endl
         << "Usage: " << strApplicationName << " [options]" << endl
         << endl
         << "Options:" << endl
         << "    --help, -h:         Display this help" << endl
         << "    --host=<host>:      The host name or IP address that the server must listen on." << endl
         << "                        If not specified, the first available is used." << endl
         << "    --port=<port>:      The port that the server must listen on (default: 11200)" << endl
         << "    --config=<file>:    Configuration file to use (default: When executed from the build" << endl
         << "                        directory of the Framework, ${FRAMEWORK_SRC}/application-servers/goalplanning-simulator/config.txt." << endl
         << "                        Otherwise, config.txt)" << endl
         << "    --logfolder=<path>: Path to the location of the log files (default: 'logs/')" << endl
         << "    --verbose:          Verbose output" << endl;
}


int main(int argc, char** argv)
{
    // Declarations
    bool            bVerbose = false;
    string          strHost = "";
    unsigned int    port = 11200;
    string          strConfig = "";

    // Determine if we are running from the build directory
    struct stat fileInfo;
    if (stat("MASH_FRAMEWORK", &fileInfo) == 0)
        strConfig = MASH_SOURCE_DIR "application-servers/goalplanning-simulator/config.txt";
    else
        strConfig = "config.txt";

    // Parse the command-line arguments
    CSimpleOpt args(argc, argv, COMMAND_LINE_OPTIONS);
    while (args.Next())
    {
        if (args.LastError() == SO_SUCCESS)
        {
            switch (args.OptionId())
            {
                case OPT_HELP:
                    showUsage(argv[0]);
                    return 0;
                
                case OPT_HOST:
                    strHost = args.OptionArg();
                    break;

                case OPT_PORT:
                    port = StringUtils::parseUnsignedInt(args.OptionArg());
                    break;

                case OPT_CONFIG:
                    strConfig = args.OptionArg();
                    break;

                case OPT_LOG_FOLDER:
                    Server::strLogFolder = args.OptionArg();
                    if (Server::strLogFolder[Server::strLogFolder.size() - 1] != '/')
                        Server::strLogFolder += "/";
                    break;

                case OPT_VERBOSE:
                    bVerbose = true;
                    break;
            }
        }
        else
        {
            cerr << "Invalid argument: " << args.OptionText() << endl;
            return -1;
        }
    }


    // Load the configuration file
    CommandsSerializer serializer;
    if (!serializer.deserialize(strConfig))
    {
        cerr << "Failed to read the configuration file" << endl;
        return -1;
    }

    for (unsigned int i = 0; i < serializer.nbCommands(); ++i)
    {
        CommandsSerializer::tCommand command = serializer.getCommand(i);
        
        GoalPlanningSimulator::tEnvironment environment;
        tStringList goals;

        if (command.strCommand == "RECORDING")
        {
            goals.push_back(command.arguments.getString(0));
            
            environment.name = command.arguments.getString(1);
            environment.url = command.arguments.getString(2);
            environment.recording = true;
        }
        else
        {
            environment.name = command.arguments.getString(0);
            environment.url = command.arguments.getString(1);
            environment.recording = false;

            goals.push_back("ReachRedCube");
            goals.push_back("ReachBlueCylinder");
            goals.push_back("ReachGreenCylinder");
            goals.push_back("ReachYellowCube");
            goals.push_back("ReachYellowCylinder");
            goals.push_back("ReachRedCube-randomized");
            goals.push_back("ReachBlueCylinder-randomized");
            goals.push_back("ReachGreenCylinder-randomized");
            goals.push_back("ReachYellowCube-randomized");
            goals.push_back("ReachYellowCylinder-randomized");
        }

        for (unsigned int i = 0; i < goals.size(); ++i)
        {
            string goal = goals[i];

            GoalPlanningSimulator::tTaskIterator iter = GoalPlanningSimulator::TASKS.find(goal);
            if (iter != GoalPlanningSimulator::TASKS.end())
            {
                iter->second.push_back(environment);
            }
            else
            {
                GoalPlanningSimulator::tEnvironmentList environments;
                environments.push_back(environment);
                GoalPlanningSimulator::TASKS[goal] = environments;
            }
        }
    }


    InteractiveApplicationServer server;

    cout << "********************************************************************************" << endl
         << "* Goal-planning Simulator" << endl
         << "* Protocol: " << server.getProtocol() << endl
         << "********************************************************************************" << endl
         << endl;

    // Start the server
    return (server.listen(strHost, port, GoalPlanningSimulator::create, bVerbose) ? 0 : -1);
}
