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


/** @file   declarations.h
    @author Philip Abbet (philip.abbet@idiap.ch)

    Declarations used all over mash-sandboxing
*/

#ifndef _MASHSANDBOXING_DECLARATIONS_H_
#define _MASHSANDBOXING_DECLARATIONS_H_

#include <sys/time.h>


#ifndef MASH_CORE_DUMP_TEMPLATE
#define MASH_CORE_DUMP_TEMPLATE "core"
#endif


namespace Mash
{
    //--------------------------------------------------------------------------
    /// @brief  Contains the configuration of a sandbox
    //--------------------------------------------------------------------------
    struct tSandboxConfiguration
    {
        tSandboxConfiguration()
        : verbosity(0), strCoreDumpTemplate(MASH_CORE_DUMP_TEMPLATE), strUsername(""), strJailDir("jail/"),
          strLogDir("logs/"), strOutputDir("out/"), strScriptsDir("./"), strTempDir("./"),
          strSourceDir(""), bDeleteAllLogFiles(true)
        {
        }

        unsigned int    verbosity;              ///< Level of verbosity
        std::string     strCoreDumpTemplate;    ///< Template of the name of the core dump files
        std::string     strUsername;            ///< User for the untrusted plugins
        std::string     strJailDir;             ///< Jail folder
        std::string     strLogDir;              ///< Log folder
        std::string     strOutputDir;           ///< Output folder (for the data files)
        std::string     strScriptsDir;          ///< The directory in which the 'coredump_analyzer.py' script is located
        std::string     strTempDir;             ///< The temporary directory for the sandbox
        std::string     strSourceDir;           ///< Directory containing the source code of the untrusted plugins
        bool            bDeleteAllLogFiles;     ///< Indicates if all the log files must be deleted at shutdown
    };


    //--------------------------------------------------------------------------
    /// @brief  Contains the declarations related to the sandbox controllers
    //--------------------------------------------------------------------------
    namespace SandboxControllerDeclarations
    {
        const unsigned int TIMEOUT_SANDBOX_CREATION  = 5000;
        const unsigned int TIMEOUT_SANDBOX           = 30000;


        enum tCommandProcessingResult
        {
            COMMAND_PROCESSED,
            COMMAND_UNKNOWN,
            SOURCE_PLUGIN_CRASHED,
            DEST_PLUGIN_CRASHED,
            DEST_PLUGIN_TIMEOUT,
            INVALID_ARGUMENTS,
            APPSERVER_ERROR,
        };
    }


    //--------------------------------------------------------------------------
    /// @brief  Contains the declarations related to the time budgets
    //--------------------------------------------------------------------------
    namespace SandboxTimeBudgetDeclarations
    {
        const struct timeval BUDGET_INITIALIZATION = { 2, 0 };
        const struct timeval BUDGET_PER_SEQUENCE   = { 0, 500 * 1e3 };
        const struct timeval BUDGET_PER_PIXEL      = { 0, 1 };
        const struct timeval BUDGET_PER_FEATURE    = { 0, 10 };
        const struct timeval BUDGET_MAXIMUM        = { 1, 0 };          // = BUDGET_PER_PIXEL * 1000 * 1000
        const struct timeval ADDITIONAL_TIMEOUT    = { 5, 0 };
    }

    
    //--------------------------------------------------------------------------
    /// @brief  Contains the statistics about a type of event that happens
    ///         during the usage of a heuristic
    //--------------------------------------------------------------------------
    struct tStatisticsEntry
    {
        tStatisticsEntry()
        : nb_events(0)
        {
            timerclear(&total_duration);
            timerclear(&mean_duration);
        }
        
        struct timeval  total_duration;
        unsigned int    nb_events;
        struct timeval  mean_duration;
    };


    //--------------------------------------------------------------------------
    /// @brief  Contains the statistics about a type of event (which can be
    ///         decomposed in subevents) that happens during the usage of a
    ///         heuristic
    //--------------------------------------------------------------------------
    struct tStatisticsComplexEntry
    {
        tStatisticsComplexEntry()
        : nb_events(0), nb_subevents(0)
        {
            timerclear(&total_duration);
            timerclear(&mean_event_duration);
            timerclear(&mean_subevent_duration);
        }

        struct timeval  total_duration;
        unsigned int    nb_events;
        unsigned int    nb_subevents;
        struct timeval  mean_event_duration;
        struct timeval  mean_subevent_duration;
    };


    //--------------------------------------------------------------------------
    /// @brief  Contains the statistics about the usage of a heuristic
    //--------------------------------------------------------------------------
    struct tHeuristicStatistics
    {
        tHeuristicStatistics()
        {
            timerclear(&total_duration);
        }
        
        tStatisticsEntry            initialization;
        tStatisticsEntry            sequences;
        tStatisticsComplexEntry     images;
        tStatisticsComplexEntry     positions;
        tStatisticsEntry            features;
        struct timeval              total_duration;
    };
}

#endif
