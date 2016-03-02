#include "sandbox.h"
#include <mash-utils/stringutils.h>
#include <SimpleOpt.h>
#include <iostream>

using namespace std;
using namespace Mash;


/**************************** COMMAND-LINE PARSING ****************************/

enum tOptions
{
    OPT_USERNAME,
    OPT_LOG_FOLDER,
    OPT_LOG_SUFFIX,
    OPT_OUTPUT_FOLDER,
    OPT_JAIL_FOLDER,
    OPT_COREDUMP_FOLDER,
    OPT_READ_FD,
    OPT_WRITE_FD,
    OPT_VERBOSE,
    OPT_VERBOSE1,
    OPT_VERBOSE2,
    OPT_VERBOSE3,
    OPT_VERBOSE4,
    OPT_VERBOSE5,
    OPT_HELP,
};

CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] =
{
    { OPT_USERNAME,             "--username",       SO_REQ_CMB },
    { OPT_LOG_FOLDER,           "--logfolder",      SO_REQ_CMB },
    { OPT_LOG_SUFFIX,           "--logsuffix",      SO_REQ_CMB },
    { OPT_OUTPUT_FOLDER,        "--outputfolder",   SO_REQ_CMB },
    { OPT_JAIL_FOLDER,          "--jailfolder",     SO_REQ_CMB },
    { OPT_READ_FD,              "--readfd",         SO_REQ_CMB },
    { OPT_WRITE_FD,             "--writefd",        SO_REQ_CMB },
    { OPT_VERBOSE,              "--verbose",        SO_NONE    },
    { OPT_VERBOSE1,             "-v",               SO_NONE    },
    { OPT_VERBOSE2,             "-vv",              SO_NONE    },
    { OPT_VERBOSE3,             "-vvv",             SO_NONE    },
    { OPT_VERBOSE4,             "-vvvv",            SO_NONE    },
    { OPT_VERBOSE5,             "-vvvvv",           SO_NONE    },
    { OPT_HELP,                 "--help",           SO_NONE    },
    { OPT_HELP,                 "-h",               SO_NONE    },
    SO_END_OF_OPTIONS
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "MASH Sandbox" << endl
         << "Usage: " << strApplicationName << " [options] { heuristics | classifier | goalplanner | instruments }" << endl
         << endl
         << "Creates a sandboxed environment when untrusted plugins can be safely" << endl
         << "executed." << endl
         << endl
         << "An untrusted plugin can be a heuristic, a classifier, a goalplanner or an instrument." << endl
         << endl
         << "This program is designed to work in collaboration with an Experiment Server," << endl
         << "not as a standalone program." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h:             Display this help" << endl
         << "    --username=<username>:  The untrusted plugins will be run by this user. Mandatory" << endl
         << "                            if the sandbox is run as root." << endl
         << "    --logfolder=<DIR>:      Path to the location of the log files (default: 'logs/')" << endl
         << "    --logsuffix=<SUFFIX>:   Suffix to use in the name of the log files (default: '')" << endl
         << "    --outputfolder=<DIR>:   Path to the location of the files written by the untrusted" << endl
         << "                            plugins, if applicable (default: 'out/')" << endl
         << "    --jailfolder=<DIR>:     Path to the directory that will serve as the root ('/') of the" << endl
         << "                            filesystem for the untrusted plugins. The sandbox must be run" << endl
         << "                            as root. (default: 'jail')" << endl
         << "    --readfd=<FD>," << endl
         << "    --writefd=<FD>:         The file descriptors to use to communicate with the Experiment" << endl
         << "                            Server (required)" << endl
         << "    --verbose," << endl
         << "    -v, -vv, -vvv, -vvvv, -vvvvv:" << endl
         << "                            Verbose output" << endl;
}


int main(int argc, char** argv)
{
    // Declarations
    Sandbox::tConfiguration configuration;
    Sandbox                 sandbox;

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
                
                case OPT_USERNAME:
                    configuration.strUsername = args.OptionArg();
                    break;

                case OPT_LOG_FOLDER:
                    configuration.strLogFolder = args.OptionArg();
                    break;

                case OPT_LOG_SUFFIX:
                    configuration.strLogSuffix = args.OptionArg();
                    break;

                case OPT_OUTPUT_FOLDER:
                    configuration.strOutputFolder = args.OptionArg();
                    break;

                case OPT_JAIL_FOLDER:
                    configuration.strJailFolder = args.OptionArg();
                    break;

                case OPT_READ_FD:
                    configuration.read_pipe = StringUtils::parseInt(args.OptionArg());
                    break;

                case OPT_WRITE_FD:
                    configuration.write_pipe = StringUtils::parseInt(args.OptionArg());
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
            }
        }
        else
        {
            cerr << "Invalid argument: " << args.OptionText() << endl;
            return -1;
        }
    }

    if (args.FileCount() != 1)
    {
        cerr << "Missing argument: the kind of sandbox (heuristics, classifier, goalplanner, instruments)" << endl;
        showUsage(argv[0]);
        return -1;
    }
    
    string strType = args.File(0);
    if (strType == "heuristics")
        configuration.kind = Sandbox::KIND_HEURISTICS;
    else if (strType == "classifier")
        configuration.kind = Sandbox::KIND_CLASSIFIER;
    else if (strType == "goalplanner")
        configuration.kind = Sandbox::KIND_GOALPLANNER;
    else if (strType == "instruments")
        configuration.kind = Sandbox::KIND_INSTRUMENTS;

    // Initialize the sandbox
    bool result = sandbox.init(configuration);
    if (!result)
        return -1;

    // Run the sandbox
    return (sandbox.run() ? 1 : 0);
}
