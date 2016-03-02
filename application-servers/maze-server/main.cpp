#include "maze_server.h"
#include <mash-appserver/interactive_application_server.h>
#include <mash-utils/stringutils.h>
#include <SimpleOpt.h>
#include <iostream>

using namespace Mash;
using namespace std;


/**************************** COMMAND-LINE PARSING ****************************/

enum tOptions
{
    OPT_HOST,
    OPT_PORT,
    OPT_LOG_FOLDER,
    OPT_VERBOSE,
    OPT_HELP,
};

CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] =
{
    { OPT_HOST,         "--host",       SO_REQ_CMB },
    { OPT_PORT,         "--port",       SO_REQ_CMB },
    { OPT_LOG_FOLDER,   "--logfolder",  SO_REQ_CMB },
    { OPT_VERBOSE,      "--verbose",    SO_NONE    },
    { OPT_HELP,         "--help",       SO_NONE    },
    { OPT_HELP,         "-h",           SO_NONE    },
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "MASH Maze Server" << endl
         << "Usage: " << strApplicationName << " [options]" << endl
         << endl
         << "Options:" << endl
         << "    --help, -h:         Display this help" << endl
         << "    --host=<host>:      The host name or IP address that the server must listen on." << endl
         << "                        If not specified, the first available is used." << endl
         << "    --port=<port>:      The port that the server must listen on (default: 11100)" << endl
         << "    --logfolder=<path>: Path to the location of the log files (default: 'logs/')" << endl
         << "    --verbose:          Verbose output" << endl;
}


int main(int argc, char** argv)
{
    // Declarations
    bool            bVerbose = false;
    string          strHost = "";
    unsigned int    port = 11100;


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


    InteractiveApplicationServer server;

    cout << "********************************************************************************" << endl
         << "* Maze Server" << endl
         << "* Protocol: " << server.getProtocol() << endl
         << "********************************************************************************" << endl
         << endl;

    // Start the server
    return (server.listen(strHost, port, MazeServer::create, bVerbose) ? 0 : -1);
}
