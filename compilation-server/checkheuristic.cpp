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


/** @file   checkheuristic.cpp
    @author Philip Abbet (philip.abbet@idiap.ch)

    Program that check that a heuristic executes correctly, part of the MASH
    Compilation Server
*/

#include <iostream>
#include <fstream>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <assert.h> 

#include <mash/sandboxed_heuristics_set.h>
#include <mash/image.h>
#include <mash/imageutils.h>
#include <mash-utils/errors.h>
#include <SimpleOpt.h>
#include <FreeImage.h>

#if USE_CURL
    #include <mash/curl_image_downloader.h>
#endif


using namespace std;
using namespace Mash;


/**************************** COMMAND-LINE PARSING ****************************/

// The valid options
enum
{
    OPT_HELP,
    OPT_VERBOSE,
    OPT_HEURISTICS_DIR,
    OPT_DATA_DIR,
    OPT_LOG_DIR,
    OPT_CORE_DUMP_TEMPLATE,
    OPT_SANDBOX_USERNAME,
    OPT_SANDBOX_JAIL_DIR,
    OPT_SANDBOX_SCRIPTS_DIR,
    OPT_SANDBOX_TEMP_DIR,
    OPT_SANDBOX_SOURCE_HEURISTICS,
};


const CSimpleOpt::SOption COMMAND_LINE_OPTIONS[] = {
    { OPT_HELP,                         "-h",                   SO_NONE    },
    { OPT_HELP,                         "--help",               SO_NONE    },
    { OPT_HEURISTICS_DIR,               "--heuristicsdir",      SO_REQ_CMB },
    { OPT_DATA_DIR,                     "--datadir",            SO_REQ_CMB },
    { OPT_LOG_DIR,                      "--logdir",             SO_REQ_CMB },
    { OPT_CORE_DUMP_TEMPLATE,           "--coredump-template",  SO_REQ_CMB },
    { OPT_SANDBOX_USERNAME,             "--sandbox-username",   SO_REQ_CMB },
    { OPT_SANDBOX_JAIL_DIR,             "--sandbox-jaildir",    SO_REQ_CMB },
    { OPT_SANDBOX_SCRIPTS_DIR,          "--sandbox-scriptsdir", SO_REQ_CMB },
    { OPT_SANDBOX_TEMP_DIR,             "--sandbox-tempdir",    SO_REQ_CMB },
    { OPT_SANDBOX_SOURCE_HEURISTICS,    "--source-heuristics",  SO_REQ_CMB },
    
    SO_END_OF_OPTIONS
};


// The files
enum
{
    FILE_HEURISTIC,
    FILE_IMAGE,

    FILES_COUNT
};


/********************************** FUNCTIONS *********************************/

void showUsage(const std::string& strApplicationName)
{
    cout << "MASH Heuristic Checker" << endl
         << "Usage: " << strApplicationName << " [options] <heuristic name> <image name>" << endl
         << endl
         << "With:" << endl
         << "    <heuristic name>:        Name of the heuristic to check." << endl
         << "    <image name>:            Path to the image file that will be processed by" << endl
         << "                             the heuristic. The path can be relative to the" << endl
         << "                             'data' folder." << endl
         << endl
         << "Options:" << endl
         << "    --help, -h:              Display this help" << endl
         << "    --heuristicsdir=<PATH>   Path to the compiled heuristics" << endl
         << "    --datadir=<PATH>         Path to the data" << endl
         << "    --logdir=<PATH>          Path to the log files" << endl
         << "    --coredump-template=<TEMPLATE>:" << endl
         << "                             Template of the name of the core dump files" << endl
         << "    --sandbox-username=<NAME>:" << endl
         << "                             The untrusted plugins in the sandboxes will be run by this user." << endl
         << "                             Mandatory if the sandbox is run as root." << endl
         << "    --sandbox-jaildir=<DIR>: Path to the directory in which the folders that will serve" << endl
         << "                             as the root ('/') of the filesystem for the sandboxes will be" << endl
         << "                             created" << endl
         << "    --sandbox-scriptsdir=<DIR>:" << endl
         << "                             Path to the directory where the scripts used for the" << endl
         << "                             sandboxing ('coredump_analyzer.py' and the 'get_stack*.cmd'" << endl
         << "                             files) are located" << endl
         << "    --sandbox-tempdir=<DIR>: Path to the directory to use to write temporary files" << endl
         << "                             during core dump analysis" << endl
         << "    --source-heuristics=<DIR>:" << endl
         << "                             Path to the directory where the source code file of the" << endl
         << "                             heuristic is located." << endl
         << endl
         << "This program is intended to be used in conjonction with the Compilation Server." << endl;
}


void reportErrors(SandboxedHeuristicsSet* pHeuristicsSet)
{
    // Assertions
    assert(pHeuristicsSet);
    
    // Test if the heuristic crashed
    tError heuristic_error = pHeuristicsSet->getLastError();

    if (heuristic_error == ERROR_CHANNEL_SLAVE_CRASHED)
        heuristic_error = ERROR_HEURISTIC_CRASHED;
    
    if (heuristic_error == ERROR_HEURISTIC_CRASHED)
    {
        cerr << "HEURISTIC_CRASH" << endl;
        cerr << "<mash:context>" << pHeuristicsSet->getContext() << "</mash:context>" << endl;

        string strStackTrace = pHeuristicsSet->getStackTrace();
        if (!strStackTrace.empty())
            cerr << "<mash:stacktrace>" << strStackTrace << "</mash:stacktrace>" << endl;
    }
    
    // Test if a heuristic exhausted its time budget
    else if (heuristic_error == ERROR_HEURISTIC_TIMEOUT)
    {
        cerr << "HEURISTIC_TIMEOUT" << endl;
        cerr << "<mash:context>" << pHeuristicsSet->getContext() << "</mash:context>" << endl;
    }
    
    // Other heuristic errors
    else if (heuristic_error != ERROR_NONE)
    {
        cerr << "HEURISTIC_ERROR" << endl;
        cerr << "<mash:error>" << getErrorDetailedDescription(heuristic_error) << "</mash:error>" << endl;
        cerr << "<mash:context>" << pHeuristicsSet->getContext() << "</mash:context>" << endl;
    }
}


int main(int argc, char** argv)
{
    // Declarations
    coordinates_t           coordinates;
    unsigned int            roi_extent          = 0;
    Image*                  pImage              = 0;
    SandboxedHeuristicsSet  heuristicsSet;
    string                  strHeuristicsDir;
    string                  strDataDir;
    tSandboxConfiguration   configuration;

    OutStream::verbosityLevel = 3;
    configuration.verbosity = 3;


    // Parse the command-line parameters
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

                case OPT_HEURISTICS_DIR:
                    strHeuristicsDir = args.OptionArg();
                    if (strHeuristicsDir.at(strHeuristicsDir.length() - 1) != '/')
                        strHeuristicsDir += "/";
                    break;

                case OPT_DATA_DIR:
                    strDataDir = args.OptionArg();
                    if (strDataDir.at(strDataDir.length() - 1) != '/')
                        strDataDir += "/";
                    break;
                    
                case OPT_LOG_DIR:
                    configuration.strLogDir = args.OptionArg();
                    break;

                case OPT_CORE_DUMP_TEMPLATE:
                    configuration.strCoreDumpTemplate = args.OptionArg();
                    break;

                case OPT_SANDBOX_USERNAME:
                    configuration.strUsername = args.OptionArg();
                    break;

                case OPT_SANDBOX_JAIL_DIR:
                    configuration.strJailDir = args.OptionArg();
                    if (configuration.strJailDir.at(configuration.strJailDir.length() - 1) != '/')
                        configuration.strJailDir += "/";
                    break;

                case OPT_SANDBOX_SCRIPTS_DIR:
                    configuration.strScriptsDir = args.OptionArg();
                    if (configuration.strScriptsDir.at(configuration.strScriptsDir.length() - 1) != '/')
                        configuration.strScriptsDir += "/";
                    break;

                case OPT_SANDBOX_TEMP_DIR:
                    configuration.strTempDir = args.OptionArg();
                    if (configuration.strTempDir.at(configuration.strTempDir.length() - 1) != '/')
                        configuration.strTempDir += "/";
                    break;

                case OPT_SANDBOX_SOURCE_HEURISTICS:
                    configuration.strSourceDir = args.OptionArg();
                    if (configuration.strSourceDir.at(configuration.strSourceDir.length() - 1) != '/')
                        configuration.strSourceDir += "/";
                    break;
            }
        }
        else
        {
            cerr << "Invalid argument: " << args.OptionText() << endl;
            return -1;
        }
    }

    if (args.FileCount() != FILES_COUNT)
    {
        cerr << "Invalid files list" << endl;
        showUsage(argv[0]);
        return -1;
    }


#if USE_CURL
    ImageUtils::setDownloader(new CURLImageDownloader());
#endif


    // Load the image
    string strImagePath = args.File(FILE_IMAGE);
    struct stat fileInfo;
    if (stat(strImagePath.c_str(), &fileInfo) != 0)
        strImagePath = strDataDir + "/" + args.File(FILE_IMAGE);
    
    pImage = ImageUtils::loadImage(strImagePath);
    if (!pImage)
    {
        cerr << "Failed to open the image '" << args.File(FILE_IMAGE) << "'" << endl;
        return -1;
    }


    // Compute the coordinates of the point to process and the extent of the
    // region of interest
    unsigned int image_dim = std::min(pImage->width(), pImage->height());
    if (image_dim % 2 == 0)
        --image_dim;
    
    roi_extent = (image_dim - 1) >> 1;
    unsigned int roi_size = roi_extent * 2 + 1;
    
    coordinates.x = roi_extent + ((pImage->width() - roi_size) >> 1);
    coordinates.y = roi_extent + ((pImage->height() - roi_size) >> 1);

    // Create the sandbox
    if (!heuristicsSet.createSandbox(configuration) || !heuristicsSet.setHeuristicsFolder(strHeuristicsDir))
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }

    // Load the heuristic
    if ((heuristicsSet.loadHeuristicPlugin(args.File(FILE_HEURISTIC)) < 0) || !heuristicsSet.createHeuristics())
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }


    // Add the missing pixel formats to the image
    ImageUtils::convertImageToPixelFormats(pImage, Image::PIXELFORMAT_ALL);


    // Initialize the heuristic
    if (!heuristicsSet.init(0, 1, roi_extent))
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }


    // Retrieve the dimension of the heuristic
    unsigned int dim = heuristicsSet.dim(0);
    if (dim == 0)
    {
        if (heuristicsSet.getLastError() == ERROR_NONE)
        {
            cerr << "HEURISTIC_ERROR" << endl;
            cerr << "<mash:error>Invalid heuristic dimension: the heuristic doesn't report any features. Please implement the dim() method.</mash:error>" << endl;
            cerr << "<mash:context>" << heuristicsSet.getContext() << "</mash:context>" << endl;
        }
        else
        {
            reportErrors(&heuristicsSet);
        }

        delete pImage;
        return -1;
    }


    // Ask the heuristic to begin a new sequence
    if (!heuristicsSet.prepareForSequence(0))
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }


    // Ask the heuristic to preprocess the image
    if (!heuristicsSet.prepareForImage(0, 0, 0, pImage))
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }


    // Ask the heuristic to preprocess the ROI
    if (!heuristicsSet.prepareForCoordinates(0, coordinates))
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }


    // Compute all the features of the heuristic
    const unsigned int NB_MAX_FEATURES = 10000;
    unsigned int indices[NB_MAX_FEATURES];
    scalar_t results[NB_MAX_FEATURES];
    unsigned int i = 0;
    while (i < dim)
    {
        unsigned int nbFeatures = 0;
        for (nbFeatures = 0; (nbFeatures < NB_MAX_FEATURES) && (i + nbFeatures < dim); ++nbFeatures)
            indices[nbFeatures] = i + nbFeatures;

        if (!heuristicsSet.computeSomeFeatures(0, nbFeatures, indices, results))
        {
            reportErrors(&heuristicsSet);
            delete pImage;
            return -1;
        }
        
        i += nbFeatures;
    }
    

    // Let the heuristic release the memory it allocated
    if (!heuristicsSet.finishForCoordinates(0))
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }

    if (!heuristicsSet.finishForImage(0))
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }

    if (!heuristicsSet.finishForSequence(0))
    {
        reportErrors(&heuristicsSet);
        delete pImage;
        return -1;
    }


    cout << "Test terminated correctly." << endl;


    // Cleanup
    delete pImage;

    return 0;
}
