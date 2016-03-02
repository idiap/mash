# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-network isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR} ${MASH_SOURCE_DIR}/tests)

# List the source files (each one define an executable)
set(APPLICATION_SRCS echo_client.cpp
                     echo_server.cpp
                     binary_server.cpp
                     binary_client.cpp
                     timeout_server.cpp
)

# Create a target for each executable
foreach (APPLICATION_SRC ${APPLICATION_SRCS})

	string(REPLACE ".cpp" "" APPLICATION ${APPLICATION_SRC})

    # Create the target
	add_executable(${APPLICATION} ${APPLICATION_SRC})
	add_dependencies(${APPLICATION} mash-network run-unittests-mash-network)

	target_link_libraries(${APPLICATION} mash-network)

    get_target_property(OUTPUT_DIRECTORY ${APPLICATION} RUNTIME_OUTPUT_DIRECTORY)

    set_target_properties(${APPLICATION} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/tests_mashnetwork"
                                                    INSTALL_RPATH "."
                                                    BUILD_WITH_INSTALL_RPATH ON
                                                    COMPILE_FLAGS "-fPIC")

endforeach()


# Create the echo tests
add_test("mash-network-echo-cpp" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${OUTPUT_DIRECTORY}/tests_mashnetwork/echo_server" "${OUTPUT_DIRECTORY}/tests_mashnetwork/echo_client" "${OUTPUT_DIRECTORY}")
add_test("mash-network-echo-py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_server.py ${MASH_SOURCE_DIR}" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_client.py ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}")
add_test("mash-network-echo-py2cpp" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${OUTPUT_DIRECTORY}/tests_mashnetwork/echo_server" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_client.py ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}")
add_test("mash-network-echo-cpp2py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_server.py ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}/tests_mashnetwork/echo_client" "${OUTPUT_DIRECTORY}")

# Create the busy tests
add_test("mash-network-busy-cpp" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/busy_test.py" "${OUTPUT_DIRECTORY}/tests_mashnetwork/echo_server" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/busy_client.py ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}")
add_test("mash-network-busy-py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/busy_test.py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_server.py ${MASH_SOURCE_DIR}" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/busy_client.py ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}")

# Create the binary tests
add_test("mash-network-binary-cpp" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${OUTPUT_DIRECTORY}/tests_mashnetwork/binary_server" "${OUTPUT_DIRECTORY}/tests_mashnetwork/binary_client" "${OUTPUT_DIRECTORY}")
add_test("mash-network-binary-py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/binary_server.py ${MASH_SOURCE_DIR}" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/binary_client.py ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}")
add_test("mash-network-binary-py2cpp" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${OUTPUT_DIRECTORY}/tests_mashnetwork/binary_server" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/binary_client.py ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}")
add_test("mash-network-binary-cpp2py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/binary_server.py ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}/tests_mashnetwork/binary_client" "${OUTPUT_DIRECTORY}")

# Create the timeout tests
add_test("mash-network-timeout" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/echo_test.py" "${OUTPUT_DIRECTORY}/tests_mashnetwork/timeout_server" "${MASH_SOURCE_DIR}/tests/tests_mashnetwork/timeout_client.py  ${MASH_SOURCE_DIR}" "${OUTPUT_DIRECTORY}")
