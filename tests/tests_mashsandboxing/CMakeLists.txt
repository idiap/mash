# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-sandboxing isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR} ${MASH_SOURCE_DIR}/tests)

# List the source files (each one define a test)
set(TESTS_SRCS testCommunicationChannel_NewChannelIsGood.cpp
               testCommunicationChannel_MasterToSlaveCommunication.cpp
               testCommunicationChannel_SlaveToMasterCommunication.cpp
               testCommunicationChannel_CopiedChannelIsGood.cpp
               testCommunicationChannel_CopiedChannelShareTheirBuffer.cpp
               testCommunicationChannel_SmallPacket.cpp
               testCommunicationChannel_LargePacket.cpp
               testCommunicationChannel_MultiplePackets.cpp
               testCommunicationChannel_IncompletePacketHeader.cpp
               testCommunicationChannel_MasterDontTolerateSlaveTimeouts.cpp
)

# Create a target for each test
foreach (TEST_SRC ${TESTS_SRCS})

	string(REPLACE ".cpp" "" TEST ${TEST_SRC})

    # Create the target
	add_executable(${TEST} ${TEST_SRC})
	add_dependencies(${TEST} mash-sandboxing run-unittests-mashsandboxing)

	target_link_libraries(${TEST} mash-sandboxing dl)

    get_target_property(OUTPUT_DIRECTORY ${TEST} RUNTIME_OUTPUT_DIRECTORY)

    set_target_properties(${TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/tests_mashsandboxing"
                                             INSTALL_RPATH "."
                                             BUILD_WITH_INSTALL_RPATH ON
                                             COMPILE_FLAGS "-fPIC")

    # Create the test
	add_test(${TEST} "bash" "-c" "cd ${OUTPUT_DIRECTORY}; ./tests_mashsandboxing/${TEST}")
                        
endforeach()
