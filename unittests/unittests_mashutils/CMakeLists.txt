# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR}/dependencies/UnitTest++/src ${MASH_SOURCE_DIR})

# List the source files
set(SRCS main.cpp
         testArgumentsList.cpp
         testDataReader.cpp
         testDataWriter.cpp
         testOutStream.cpp
         testRandomNumberGenerator.cpp
         testStringUtils.cpp
)

# Create and link the executable
add_executable(unittests_mashutils ${SRCS})
target_link_libraries(unittests_mashutils UnitTest++ mash-utils)

if (NOT WIN32)
    set_target_properties(unittests_mashutils PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

# Run the unit tests
get_property(PATH TARGET unittests_mashutils PROPERTY RUNTIME_OUTPUT_DIRECTORY)
add_custom_target(run-unittests-mashutils ALL unittests_mashutils DEPENDS unittests_mashutils WORKING_DIRECTORY ${PATH} COMMENT "Unit testing: mash-utils..." VERBATIM)
