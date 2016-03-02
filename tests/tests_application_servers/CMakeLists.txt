# Create the tests
add_test("image-server" "${MASH_SOURCE_DIR}/tests/tests_application_servers/test-image-server.py" "--pymash=${MASH_SOURCE_DIR}/pymash" "--server=${MASH_SOURCE_DIR}/application-servers/image-server/image-server.py" "--serverconfig=${MASH_SOURCE_DIR}/tests/tests_application_servers/image-server-config" "127.0.0.1" "11010")
add_test("maze-server" "${MASH_SOURCE_DIR}/tests/tests_application_servers/test-maze-server.py" "--pymash=${MASH_SOURCE_DIR}/pymash" "--server=${MASH_BINARY_DIR}/bin/maze-server" "11110")
