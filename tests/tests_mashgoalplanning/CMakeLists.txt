# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-goalplanning isn't part of the SDK, and isn't supported on Windows")
endif()

if (NOT MASH_ADVANCED_TESTS)
    message(FATAL_ERROR "Advanced tests aren't enabled")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR} ${MASH_SOURCE_DIR}/tests)

# List the source files (each one define a test)
set(TESTS_SRCS testSandboxedPlanner_PlannerLoading.cpp
               testSandboxedPlanner_NoConstructorPlannerLoadingFail.cpp
               testSandboxedPlanner_UnknownPlannerLoadingFail.cpp
               testSandboxedPlanner_DetectCrashInLoading.cpp
               testSandboxedPlanner_DetectCrashInConstructor.cpp
               testSandboxedPlanner_DetectCrashInSetup.cpp
               testSandboxedPlanner_DetectCrashInLearn.cpp
               testSandboxedPlanner_DetectCrashInChooseAction.cpp
               testSandboxedPlanner_DetectCrashInReportFeaturesUsed.cpp
               testSandboxedPlanner_TaskCommunication_StandardMode.cpp
               testSandboxedPlanner_TaskCommunication_RecordedTeacherMode.cpp
               testSandboxedPlanner_PerceptionCommunication.cpp
               testSandboxedPlanner_IncompleteModelLoading.cpp
               testSandboxedPlanner_ModelLoading.cpp
               testSandboxedPlanner_ModelSaving.cpp
               testSandboxedPlanner_UnknownModelLoadingFail.cpp
               testSandboxedPlanner_Notifications.cpp
               testTrustedPlanner_PlannerLoading.cpp
               testTrustedPlanner_NoConstructorPlannerLoadingFail.cpp
               testTrustedPlanner_UnknownPlannerLoadingFail.cpp
               testTrustedPlanner_TaskCommunication.cpp
               testTrustedPlanner_PerceptionCommunication.cpp
               testTrustedPlanner_IncompleteModelLoading.cpp
               testTrustedPlanner_ModelLoading.cpp
               testTrustedPlanner_ModelSaving.cpp
               testTrustedPlanner_UnknownModelLoadingFail.cpp
               testTrustedPlanner_Notifications.cpp
)

# Create a target for each test
foreach (TEST_SRC ${TESTS_SRCS})

	string(REPLACE ".cpp" "" TEST ${TEST_SRC})

    # Create the target
	add_executable(${TEST} ${TEST_SRC})
	add_dependencies(${TEST} mash-goalplanning mash-core FreeImage run-unittests-goalplanning)
	target_link_libraries(${TEST} mash-goalplanning mash-core mash-utils dl)

    get_target_property(OUTPUT_DIRECTORY ${TEST} RUNTIME_OUTPUT_DIRECTORY)

    set_target_properties(${TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/tests_mashgoalplanning"
                                             INSTALL_RPATH "."
                                             BUILD_WITH_INSTALL_RPATH ON
                                             COMPILE_FLAGS "-fPIC")

    # Create the test
	add_test(${TEST} "bash" "-c" "cd ${OUTPUT_DIRECTORY}; ./tests_mashgoalplanning/${TEST}")
                        
endforeach()
