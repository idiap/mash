# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The Experiment Server isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}
                    ${MASH_SOURCE_DIR}/dependencies
                    ${MASH_SOURCE_DIR}/dependencies/include
                    ${MASH_SOURCE_DIR}/dependencies/FreeImage)

# List the source files of experiment-server
set(SRCS main.cpp
         listener.cpp
         notifier.cpp
         task_controller.cpp
         classification_task.cpp
         goalplanning_task.cpp
)

# The following flags are used to simplify the use of the Framework
set(LOCATION_FLAGS "")

macro(add_locations_to_global_definitions LOCATIONS)
    set(DEFINITION "-D${LOCATIONS}=\"\\\"")
    set(FIRST TRUE)
    foreach(LOCATION ${${LOCATIONS}})
        if (NOT FIRST)
            set(DEFINITION "${DEFINITION};")
        else()
            set(FIRST FALSE)
        endif()

        set(DEFINITION "${DEFINITION}${LOCATION}")
    endforeach()
    set(DEFINITION "${DEFINITION}\"\\\"")

    set(LOCATION_FLAGS "${LOCATION_FLAGS} ${DEFINITION}")
endmacro()

add_locations_to_global_definitions(MASH_CLASSIFIER_LOCATIONS)
add_locations_to_global_definitions(MASH_GOALPLANNER_LOCATIONS)
add_locations_to_global_definitions(MASH_INSTRUMENT_LOCATIONS)
add_locations_to_global_definitions(MASH_HEURISTIC_LOCATIONS)


# Create and link the executable
add_executable(experiment-server ${SRCS})
add_dependencies(experiment-server mash-classification mash-goalplanning mash-instrumentation mash-network freeimage
                                   tinyxml heuristics classifiers goalplanners instruments)

target_link_libraries(experiment-server mash-classification mash-goalplanning mash-instrumentation mash-core
                                        mash-network freeimage tinyxml)

set_target_properties(experiment-server PROPERTIES INSTALL_RPATH "."
                                                   BUILD_WITH_INSTALL_RPATH ON
                                                   COMPILE_FLAGS "${LOCATION_FLAGS} -fPIC")

if (MASH_USE_CURL AND CURL_FOUND)
   include_directories(${CURL_INCLUDE_DIRS})
   target_link_libraries(experiment-server ${CURL_LIBRARIES})
   set_target_properties(experiment-server PROPERTIES COMPILE_DEFINITIONS "USE_CURL")
endif()


# Installation stuff
install(TARGETS experiment-server
        RUNTIME DESTINATION experiment-server
		CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )

install(PROGRAMS manage.py
        DESTINATION experiment-server
        CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )

install(DIRECTORY heuristics_cmake
        DESTINATION experiment-server
        CONFIGURATIONS Release
        COMPONENT "experiment-server"
        PATTERN CMakeDeclarations.* EXCLUDE
       )

install(FILES heuristics_cmake/CMakeDeclarations.install
        DESTINATION experiment-server/heuristics_cmake
        CONFIGURATIONS Release
        COMPONENT "experiment-server"
        RENAME "CMakeDeclarations.txt"
       )

install(FILES settings_example.txt
        DESTINATION experiment-server
        CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )
