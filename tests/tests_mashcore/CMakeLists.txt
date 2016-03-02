# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The tests of the library mash-core aren't part of the SDK, and aren't supported on Windows")
endif()

if (NOT MASH_ADVANCED_TESTS)
    message(FATAL_ERROR "Advanced tests aren't enabled")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR} ${MASH_SOURCE_DIR}/tests)

# List the source files (each one define a test)
set(TESTS_SRCS testSandboxedHeuristicsSet_HeuristicLoading.cpp
               testSandboxedHeuristicsSet_NoConstructorHeuristicLoadingFail.cpp
               testSandboxedHeuristicsSet_UnknownHeuristicLoadingFail.cpp
               testSandboxedHeuristicsSet_DetectCrashInLoading.cpp
               testSandboxedHeuristicsSet_DetectCrashInConstructor.cpp
               testSandboxedHeuristicsSet_DetectCrashInInit.cpp
               testSandboxedHeuristicsSet_DetectCrashInDim.cpp
               testSandboxedHeuristicsSet_DetectCrashInPrepareForSequence.cpp
               testSandboxedHeuristicsSet_DetectCrashInFinishForSequence.cpp
               testSandboxedHeuristicsSet_DetectCrashInPrepareForImage.cpp
               testSandboxedHeuristicsSet_DetectCrashInFinishForImage.cpp
               testSandboxedHeuristicsSet_DetectCrashInPrepareForCoordinates.cpp
               testSandboxedHeuristicsSet_DetectCrashInFinishForCoordinates.cpp
               testSandboxedHeuristicsSet_DetectCrashInComputeFeature.cpp
               testSandboxedHeuristicsSet_PreventCommandExecution.cpp
               testSandboxedHeuristicsSet_PreventDynlibLoading.cpp
               testSandboxedHeuristicsSet_PreventFileCreation.cpp
               testSandboxedHeuristicsSet_PreventFileOpening.cpp
               testSandboxedHeuristicsSet_PreventForks.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInConstructor.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInInit.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInDim.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInPrepareForSequence.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInFinishForSequence.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInPrepareForImage.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInFinishForImage.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInPrepareForCoordinates.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInFinishForCoordinates.cpp
               testSandboxedHeuristicsSet_DetectTimeBudgetExhaustionInComputeFeature.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInConstructor.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInInit.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInDim.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInPrepareForSequence.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInFinishForSequence.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInPrepareForImage.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInFinishForImage.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInPrepareForCoordinates.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInFinishForCoordinates.cpp
               testSandboxedHeuristicsSet_DetectTimeoutInComputeFeature.cpp
               testSandboxedHeuristicsSet_DetectNaNReturnedByComputeFeature.cpp
               testTrustedHeuristicsSet_HeuristicLoading.cpp
               testTrustedHeuristicsSet_NoConstructorHeuristicLoadingFail.cpp
               testTrustedHeuristicsSet_UnknownHeuristicLoadingFail.cpp
               testTrustedHeuristicsSet_DetectNaNReturnedByComputeFeature.cpp
)

if (NOT APPLE)
    set(TESTS_SRCS "${TESTS_SRCS}"
                    testSandboxedHeuristicsSet_PreventMemoryExhaustion.cpp
                    testSandboxedHeuristicsSet_PreventFileOpeningWhileLoading.cpp
    )
endif()


# Create a target for each test
foreach (TEST_SRC ${TESTS_SRCS})

	string(REPLACE ".cpp" "" TEST ${TEST_SRC})

    # Create the target
	add_executable(${TEST} ${TEST_SRC})
	add_dependencies(${TEST} mash-core FreeImage run-unittests-mashcore)
	target_link_libraries(${TEST} mash-core dl)

    get_target_property(OUTPUT_DIRECTORY ${TEST} RUNTIME_OUTPUT_DIRECTORY)

    set_target_properties(${TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/tests_mashcore"
                                             INSTALL_RPATH "."
                                             BUILD_WITH_INSTALL_RPATH ON
                                             COMPILE_FLAGS "-fPIC")

    # Create the test
	add_test(${TEST} "bash" "-c" "cd ${OUTPUT_DIRECTORY}; ./tests_mashcore/${TEST}")
                        
endforeach()
