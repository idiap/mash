# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-goalplanning isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}
                    ${MASH_SOURCE_DIR}/dependencies
                    ${MASH_SOURCE_DIR}/dependencies/UnitTest++/src
)

# List the source files
file(GLOB SRCS main.cpp
               testPlannersManager.cpp
)

# Create and link the executable
add_executable(unittests_mashgoalplanning ${SRCS})
add_dependencies(unittests_mashgoalplanning mash-core mash-utils mash-goalplanning)
target_link_libraries(unittests_mashgoalplanning UnitTest++ mash-goalplanning mash-core mash-utils dl)
set_target_properties(unittests_mashgoalplanning PROPERTIES COMPILE_FLAGS "-fPIC")

# Run the unit tests
get_property(PATH TARGET unittests_mashgoalplanning PROPERTY RUNTIME_OUTPUT_DIRECTORY)
add_custom_target(run-unittests-mashgoalplanning ALL unittests_mashgoalplanning
                  DEPENDS unittests_mashgoalplanning
                  WORKING_DIRECTORY ${PATH}
                  COMMENT "Unit testing: mash-goalplanning..." VERBATIM)
