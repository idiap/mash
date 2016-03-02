# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR}/dependencies/UnitTest++/src ${MASH_SOURCE_DIR})

# List the source files
set(SRCS main.cpp
         testDynlibsManager.cpp
         testHeuristicsManager.cpp
         testImage.cpp
         testImageUtils.cpp
         testImagesCache.cpp
         testPredictorModel.cpp
)

# Create and link the executable
add_executable(unittests_mashcore ${SRCS})
add_dependencies(unittests_mashcore freeimage)

if (NOT WIN32)
	target_link_libraries(unittests_mashcore UnitTest++ mash-core dl)
    set_target_properties(unittests_mashcore PROPERTIES COMPILE_FLAGS "-fPIC")
else()
	target_link_libraries(unittests_mashcore UnitTest++ mash-core)
endif()

# Run the unit tests
get_property(PATH TARGET unittests_mashcore PROPERTY RUNTIME_OUTPUT_DIRECTORY)
add_custom_target(run-unittests-mashcore ALL unittests_mashcore DEPENDS unittests_mashcore heuristics WORKING_DIRECTORY ${PATH} COMMENT "Unit testing: mashcore..." VERBATIM)
