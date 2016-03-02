# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-network isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR}/dependencies/UnitTest++/src ${MASH_SOURCE_DIR})

# List the source files
set(SRCS main.cpp
         testCommandsSerializer.cpp
)

# Create and link the executable
add_executable(unittests_mashnetwork ${SRCS})
target_link_libraries(unittests_mashnetwork UnitTest++ mash-network)
set_target_properties(unittests_mashnetwork PROPERTIES COMPILE_FLAGS "-fPIC")

# Run the unit tests
get_property(PATH TARGET unittests_mashnetwork PROPERTY RUNTIME_OUTPUT_DIRECTORY)
add_custom_target(run-unittests-mashnetwork ALL unittests_mashnetwork DEPENDS unittests_mashnetwork WORKING_DIRECTORY ${PATH} COMMENT "Unit testing: mash-network..." VERBATIM)
