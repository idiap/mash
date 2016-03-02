cmake_minimum_required(VERSION 2.6)
project(MASH)

# Policies
cmake_policy(VERSION 2.6)
cmake_policy(SET CMP0009 NEW)


# Options
option(MASH_ADVANCED_TESTS "Enable the advanced tests (when enabled, 'make test' will take several minutes to complete)" OFF)
option(MASH_USE_CURL "Enable support for CURL (to download images from a server to another one)" ON)

if (NOT APPLE)
    option(MASH_SANDBOX_MEMORY_DEBUG "Enable debugging mode for the memory monitoring in the sandbox")
endif()

set(MASH_TESTS_SANDBOX_USERNAME "sandboxed" CACHE STRING "The untrusted plugins in the sandboxes will be run by this user during testing (mandatory if the tests are run as root, not used otherwise)")

set(MASH_CLASSIFIER_LOCATIONS "${MASH_SOURCE_DIR}/classifiers;${MASH_SOURCE_DIR}/../classifiers" CACHE STRING "Locations of the classifiers, separated by ;")
set(MASH_GOALPLANNER_LOCATIONS "${MASH_SOURCE_DIR}/goalplanners;${MASH_SOURCE_DIR}/../goalplanners" CACHE STRING "Locations of the goal-planners, separated by ;")
set(MASH_INSTRUMENT_LOCATIONS "${MASH_SOURCE_DIR}/instruments;${MASH_SOURCE_DIR}/../instruments" CACHE STRING "Locations of the instruments, separated by ;")
set(MASH_HEURISTIC_LOCATIONS "${MASH_SOURCE_DIR}/heuristics;${MASH_SOURCE_DIR}/../heuristics" CACHE STRING "Locations of the heuristics, separated by ;")

if (APPLE)
    set(MASH_CORE_DUMP_TEMPLATE "/cores/core.$PID" CACHE STRING "Template of the name of the core dump files")
else()
    set(MASH_CORE_DUMP_TEMPLATE "core" CACHE STRING "Template of the name of the core dump files")
endif()

# Select a default build configuration if none was choosen
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()


# Indicates that this isn't the SDK
set(MASH_SDK false)


# Create a file in the build folder indicating that this is the build folder and not
# an installation 
file(WRITE "${MASH_BINARY_DIR}/bin/MASH_FRAMEWORK" "This file indicates that this is the build directory of the MASH Framework")


# Indicates if we are generating a project for one of the supported IDEs
if (CMAKE_GENERATOR MATCHES "(^Xcode$)|(^Visual Studio)")
	set(IDE_USED true)
else()
	set(IDE_USED false)
endif()


# Set the output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MASH_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${MASH_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${MASH_BINARY_DIR}/bin")


# Set the global compiler definitions
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELEASE "NDEBUG")

set(GLOBAL_DEFINITIONS "")

list(APPEND GLOBAL_DEFINITIONS 	"MASH_SOURCE_DIR=\"${MASH_SOURCE_DIR}/\""
                                "MASH_DATA_DIR=\"${MASH_SOURCE_DIR}/data/\""
							   	"MASH_HEURISTICS_DIR=\"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/heuristics\""
							   	"MASH_CORE_DUMP_TEMPLATE=\"${MASH_CORE_DUMP_TEMPLATE}\""
							   	"MASH_TESTS_SANDBOX_USERNAME=\"${MASH_TESTS_SANDBOX_USERNAME}\""
)

if (IDE_USED)
	list(APPEND GLOBAL_DEFINITIONS "MASH_IDE_USED")
endif()

set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${GLOBAL_DEFINITIONS}")


# Import our custom functions
include("${MASH_SOURCE_DIR}/cmake/plugins.cmake")


if (MASH_USE_CURL)
    include(FindCURL)
endif()

if (MASH_USE_CURL AND CURL_FOUND)
    message(STATUS "The MASH Framework is built with support for libcURL")
endif()


# Subdirectories to process
add_subdirectory(dependencies)
add_subdirectory(mash-utils)
add_subdirectory(mash)
add_subdirectory(sdk)
add_subdirectory(heuristics)


if (NOT WIN32)
    add_subdirectory(mash-network)
    add_subdirectory(mash-sandboxing)
    add_subdirectory(mash-classification)
    add_subdirectory(mash-goalplanning)
    add_subdirectory(mash-instrumentation)
    add_subdirectory(mash-appserver)
    add_subdirectory(application-servers)
    add_subdirectory(compilation-server)
    add_subdirectory(clustering-server)
    add_subdirectory(experiment-server)
    add_subdirectory(sandbox)
    add_subdirectory(tools)
    add_subdirectory(classifiers)
    add_subdirectory(goalplanners)
    add_subdirectory(instruments)

    # Installation stuff - Compilation Server
    install(DIRECTORY data
            DESTINATION compilation-server
            CONFIGURATIONS Release
            COMPONENT "compilation-server"
            PATTERN unittests EXCLUDE
           )

    install(DIRECTORY pymash
            DESTINATION compilation-server
            CONFIGURATIONS Release
            COMPONENT "compilation-server"
            PATTERN *.pyc EXCLUDE
           )

    # Installation stuff - Experiment Server
    install(DIRECTORY data
            DESTINATION experiment-server
            CONFIGURATIONS Release
            COMPONENT "experiment-server"
            PATTERN unittests EXCLUDE
           )

    # Installation stuff - Image Server
    install(DIRECTORY pymash
            DESTINATION image-server
            CONFIGURATIONS Release
            COMPONENT "image-server"
            PATTERN *.pyc EXCLUDE
           )

endif()


# Testing
if (NOT IDE_USED)
      enable_testing()

      add_subdirectory(unittests)
      add_subdirectory(tests)
endif()
