# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-classification isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR}/dependencies/UnitTest++/src ${MASH_SOURCE_DIR})

# List the source files
file(GLOB SRCS main.cpp
               testClassifiersManager.cpp
)

# Create and link the executable
add_executable(unittests_mashclassification ${SRCS})
add_dependencies(unittests_mashclassification mash-core freeimage mash-utils mash-classification)

target_link_libraries(unittests_mashclassification UnitTest++ mash-classification mash-core mash-utils dl)
set_target_properties(unittests_mashclassification PROPERTIES COMPILE_FLAGS "-fPIC")

# Run the unit tests
get_property(PATH TARGET unittests_mashclassification PROPERTY RUNTIME_OUTPUT_DIRECTORY)
add_custom_target(run-unittests-mashclassification ALL unittests_mashclassification
                  DEPENDS unittests_mashclassification # classifiers
                  WORKING_DIRECTORY ${PATH}
                  COMMENT "Unit testing: mash-classification..." VERBATIM)
