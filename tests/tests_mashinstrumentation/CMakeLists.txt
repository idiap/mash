# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-instrumentation isn't part of the SDK, and isn't supported on Windows")
endif()

if (NOT MASH_ADVANCED_TESTS)
    message(FATAL_ERROR "Advanced tests aren't enabled")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR} ${MASH_SOURCE_DIR}/tests)

# List the source files (each one define a test)
set(TESTS_SRCS testSandboxedInstrumentsSet_InstrumentLoading.cpp
               testSandboxedInstrumentsSet_NoConstructorInstrumentLoadingFail.cpp
               testSandboxedInstrumentsSet_UnknownInstrumentLoadingFail.cpp
               testSandboxedInstrumentsSet_DetectCrashInLoading.cpp
               testSandboxedInstrumentsSet_DetectCrashInConstructor.cpp
               testSandboxedInstrumentsSet_DetectCrashInSetup.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnClassificationExperimentStarted.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnClassifierTrainingStarted.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnClassifierTrainingDone.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnClassifierTestStarted.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnClassifierTestDone.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnClassifierClassificationDone.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnFeaturesComputedByClassifier.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnPlannerLearningStarted.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnPlannerLearningDone.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnPlannerTestStarted.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnPlannerTestDone.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnPlannerActionChoosen.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnFeaturesComputedByPlanner.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnFeatureListReported.cpp
               testSandboxedInstrumentsSet_DetectCrashInOnExperimentDone.cpp
               testSandboxedInstrumentsSet_InputSetCommunication.cpp
               testSandboxedInstrumentsSet_TaskCommunication.cpp
               testSandboxedInstrumentsSet_PerceptionCommunication.cpp
               testTrustedInstrumentsSet_InstrumentLoading.cpp
               testTrustedInstrumentsSet_NoConstructorInstrumentLoadingFail.cpp
               testTrustedInstrumentsSet_UnknownInstrumentLoadingFail.cpp
)

# Create a target for each test
foreach (TEST_SRC ${TESTS_SRCS})

	string(REPLACE ".cpp" "" TEST ${TEST_SRC})

    # Create the target
	add_executable(${TEST} ${TEST_SRC})
	add_dependencies(${TEST} mash-instrumentation mash-core FreeImage run-unittests-instrumentation)
	target_link_libraries(${TEST} mash-instrumentation mash-core mash-utils dl)

    get_target_property(OUTPUT_DIRECTORY ${TEST} RUNTIME_OUTPUT_DIRECTORY)

    set_target_properties(${TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/tests_mashinstrumentation"
                                             INSTALL_RPATH "."
                                             BUILD_WITH_INSTALL_RPATH ON
                                             COMPILE_FLAGS "-fPIC")

    # Create the test
	add_test(${TEST} "bash" "-c" "cd ${OUTPUT_DIRECTORY}; ./tests_mashinstrumentation/${TEST}")
                        
endforeach()
