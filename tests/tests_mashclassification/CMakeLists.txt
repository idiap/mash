# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-classification isn't part of the SDK, and isn't supported on Windows")
endif()

if (NOT MASH_ADVANCED_TESTS)
    message(FATAL_ERROR "Advanced tests aren't enabled")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR} ${MASH_SOURCE_DIR}/tests)

# List the source files (each one define a test)
set(TESTS_SRCS testSandboxedClassifier_ClassifierLoading.cpp
               testSandboxedClassifier_NoConstructorClassifierLoadingFail.cpp
               testSandboxedClassifier_UnknownClassifierLoadingFail.cpp
               testSandboxedClassifier_DetectCrashInLoading.cpp
               testSandboxedClassifier_DetectCrashInConstructor.cpp
               testSandboxedClassifier_DetectCrashInSetup.cpp
               testSandboxedClassifier_DetectCrashInTrain.cpp
               testSandboxedClassifier_DetectCrashInClassify.cpp
               testSandboxedClassifier_DetectCrashInReportFeaturesUsed.cpp
               testSandboxedClassifier_InputSetCommunication.cpp
               testSandboxedClassifier_UnknownModelLoadingFail.cpp
               testSandboxedClassifier_ModelLoading.cpp
               testSandboxedClassifier_ModelLoadingWithInternalData.cpp
               testSandboxedClassifier_IncompleteModelLoading.cpp
               testSandboxedClassifier_ModelSaving.cpp
               testSandboxedClassifier_ModelSavingWithInternalData.cpp
               testSandboxedClassifier_Notifications.cpp
               testTrustedClassifier_ClassifierLoading.cpp
               testTrustedClassifier_NoConstructorClassifierLoadingFail.cpp
               testTrustedClassifier_UnknownClassifierLoadingFail.cpp
               testTrustedClassifier_InputSetCommunication.cpp
               testTrustedClassifier_UnknownModelLoadingFail.cpp
               testTrustedClassifier_ModelLoading.cpp
               testTrustedClassifier_ModelLoadingWithInternalData.cpp
               testTrustedClassifier_IncompleteModelLoading.cpp
               testTrustedClassifier_ModelSaving.cpp
               testTrustedClassifier_ModelSavingWithInternalData.cpp
               testTrustedClassifier_Notifications.cpp
)

# Create a target for each test
foreach (TEST_SRC ${TESTS_SRCS})

	string(REPLACE ".cpp" "" TEST ${TEST_SRC})

    # Create the target
	add_executable(${TEST} ${TEST_SRC})
	add_dependencies(${TEST} mash-classification mash-core FreeImage run-unittests-mashclassification)
	target_link_libraries(${TEST} mash-classification mash-core mash-utils dl)

    get_target_property(OUTPUT_DIRECTORY ${TEST} RUNTIME_OUTPUT_DIRECTORY)

    set_target_properties(${TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/tests_mashclassification"
                                             INSTALL_RPATH "."
                                             BUILD_WITH_INSTALL_RPATH ON
                                             COMPILE_FLAGS "-fPIC")

    # Create the test
	add_test(${TEST} "bash" "-c" "cd ${OUTPUT_DIRECTORY}; ./tests_mashclassification/${TEST}")
                        
endforeach()
