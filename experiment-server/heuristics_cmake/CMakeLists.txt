cmake_minimum_required(VERSION 2.6)
project(EXPERIMENT_SERVER_HEURISTIC)

include(CMakeDeclarations.txt)


# Options
SET(SRC_FILE "" CACHE PATH "Source file to compile")
SET(FULL_HEURISTIC_NAME "" CACHE STRING "Name of the heuristic (format: [username/]heuristic_name[/version])")

if (SRC_FILE STREQUAL "")
    message(FATAL_ERROR "No source file option provided")
endif()

if (FULL_HEURISTIC_NAME STREQUAL "")
    message(FATAL_ERROR "No heuristic name option provided")
endif()


# Select the Release build configuration
set(CMAKE_BUILD_TYPE "RelWithDebInfo")


# Setup the search paths
include_directories(${MASH_SOURCE_DIR})
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})


# Retrieve the username and the heuristic name
set(SPLIT_REGEX_USER_NAME_VERSION "^([A-Za-z0-9_]+)/([A-Za-z0-9_]+)/([0-9]+)$")
set(SPLIT_REGEX_USER_NAME "^([A-Za-z0-9_]+)/([A-Za-z0-9_]+)$")

if (FULL_HEURISTIC_NAME MATCHES ${SPLIT_REGEX_USER_NAME_VERSION})

    string(REGEX REPLACE ${SPLIT_REGEX_USER_NAME_VERSION} "\\1" HEURISTIC_USERNAME ${FULL_HEURISTIC_NAME})
    string(REGEX REPLACE ${SPLIT_REGEX_USER_NAME_VERSION} "\\2" HEURISTIC_NAME ${FULL_HEURISTIC_NAME})
    string(REGEX REPLACE ${SPLIT_REGEX_USER_NAME_VERSION} "\\3" HEURISTIC_VERSION ${FULL_HEURISTIC_NAME})
    set(HEURISTIC_NAME "${HEURISTIC_NAME}_v${HEURISTIC_VERSION}")

else()
    
    if (FULL_HEURISTIC_NAME MATCHES ${SPLIT_REGEX_USER_NAME})
        string(REGEX REPLACE ${SPLIT_REGEX_USER_NAME} "\\1" HEURISTIC_USERNAME ${FULL_HEURISTIC_NAME})
        string(REGEX REPLACE ${SPLIT_REGEX_USER_NAME} "\\2" HEURISTIC_NAME ${FULL_HEURISTIC_NAME})
    else()
        set(HEURISTIC_USERNAME "")
        set(HEURISTIC_NAME ${FULL_HEURISTIC_NAME})
    endif()

endif()


# Create the target
add_library(${HEURISTIC_NAME} MODULE ${SRC_FILE})
target_link_libraries(${HEURISTIC_NAME} mash-core)

get_target_property(OUTPUT_DIRECTORY ${HEURISTIC_NAME} LIBRARY_OUTPUT_DIRECTORY)

set_target_properties(${HEURISTIC_NAME}
                      PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/${HEURISTICS_BUILD_DIR}/${HEURISTIC_USERNAME}"
                                 OUTPUT_NAME "${HEURISTIC_NAME}"
                                 INSTALL_RPATH "."
                                 BUILD_WITH_INSTALL_RPATH ON
                                 COMPILE_FLAGS "-fPIC")
