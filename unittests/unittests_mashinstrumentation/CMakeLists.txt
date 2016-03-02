# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR}/dependencies/UnitTest++/src ${MASH_SOURCE_DIR})

# List the source files
set(SRCS main.cpp
         testDataWriter.cpp
         testInstrumentsManager.cpp
)

# Create and link the executable
add_executable(unittests_mashinstrumentation ${SRCS})
target_link_libraries(unittests_mashinstrumentation UnitTest++ mash-instrumentation mash-utils)

if (NOT WIN32)
    set_target_properties(unittests_mashinstrumentation PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

# Run the unit tests
get_property(PATH TARGET unittests_mashinstrumentation PROPERTY RUNTIME_OUTPUT_DIRECTORY)
add_custom_target(run-unittests-mashinstrumentation ALL unittests_mashinstrumentation DEPENDS unittests_mashinstrumentation WORKING_DIRECTORY ${PATH} COMMENT "Unit testing: mash-instrumentation..." VERBATIM)
