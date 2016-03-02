# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-network isn't part of the SDK, and isn't supported on Windows")
endif()

if (NOT MASH_ADVANCED_TESTS)
    message(FATAL_ERROR "Advanced tests aren't enabled")
endif()


configure_file("${MASH_SOURCE_DIR}/tests/tests_compilation_server/compilation-server-config_template.py"
               "${MASH_SOURCE_DIR}/tests/tests_compilation_server/compilation-server-config.py")


function(add_heuristic_crash_test METHOD_NAME)
    add_test("mash-compilation-server-heuristic-crash-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_compilation_server/run.py" "crash" "unittests/crash_in_${METHOD_NAME}" "${METHOD_NAME}")
endfunction()


function(add_heuristic_timeout_test METHOD_NAME)
    add_test("mash-compilation-server-heuristic-timeout-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_compilation_server/run.py" "timeout" "unittests/timeout_in_${METHOD_NAME}" "${METHOD_NAME}")
endfunction()


function(add_heuristic_timeout_test METHOD_NAME)
    add_test("mash-compilation-server-heuristic-timeout-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_compilation_server/run.py" "timeout" "unittests/timeout_in_${METHOD_NAME}" "${METHOD_NAME}")
endfunction()


function(add_heuristic_budget_exhaustion_test METHOD_NAME)
    add_test("mash-compilation-server-heuristic-budget-exhaustion-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_compilation_server/run.py" "timeout" "unittests/time_budget_exhaustion_in_${METHOD_NAME}" "${METHOD_NAME}")
endfunction()


function(add_heuristic_error_test HEURISTIC_NAME METHOD_NAME)
    add_test("mash-compilation-server-heuristic-error-${HEURISTIC_NAME}" "${MASH_SOURCE_DIR}/tests/tests_compilation_server/run.py" "error" "unittests/${HEURISTIC_NAME}" "${METHOD_NAME}")
endfunction()


function(add_heuristic_successful_test HEURISTIC_NAME)
    add_test("mash-compilation-server-heuristic-success-${HEURISTIC_NAME}" "${MASH_SOURCE_DIR}/tests/tests_compilation_server/run.py" "none" "examples/${HEURISTIC_NAME}")
endfunction()


# Create the compilation-related tests
add_test("mash-compilation-server-heuristic-compilation-fail" "${MASH_SOURCE_DIR}/tests/tests_compilation_server/run.py" "compilation" "upload/compilationfail")


# Create the crash-related tests
add_heuristic_crash_test("constructor")
add_heuristic_crash_test("init")
add_heuristic_crash_test("dim")
add_heuristic_crash_test("prepareforsequence")
add_heuristic_crash_test("prepareforimage")
add_heuristic_crash_test("prepareforcoordinates")
add_heuristic_crash_test("computefeature")
add_heuristic_crash_test("finishforcoordinates")
add_heuristic_crash_test("finishforimage")
add_heuristic_crash_test("finishforsequence")


# Create the timeout-related tests
add_heuristic_timeout_test("constructor")
add_heuristic_timeout_test("init")
add_heuristic_timeout_test("dim")
add_heuristic_timeout_test("prepareforsequence")
add_heuristic_timeout_test("prepareforimage")
add_heuristic_timeout_test("prepareforcoordinates")
add_heuristic_timeout_test("computefeature")
add_heuristic_timeout_test("finishforcoordinates")
add_heuristic_timeout_test("finishforimage")
add_heuristic_timeout_test("finishforsequence")


# Create the budget exhaustion-related tests
add_heuristic_budget_exhaustion_test("constructor")
add_heuristic_budget_exhaustion_test("init")
add_heuristic_budget_exhaustion_test("dim")
add_heuristic_budget_exhaustion_test("prepareforsequence")
add_heuristic_budget_exhaustion_test("prepareforimage")
add_heuristic_budget_exhaustion_test("prepareforcoordinates")
add_heuristic_budget_exhaustion_test("computefeature")
add_heuristic_budget_exhaustion_test("finishforcoordinates")
add_heuristic_budget_exhaustion_test("finishforimage")
add_heuristic_budget_exhaustion_test("finishforsequence")


# Create the error-related tests
add_heuristic_error_test("noconstructor", "constructor")
add_heuristic_error_test("nodimension", "dim")


# Create the success-related tests
add_heuristic_successful_test("identity")
add_heuristic_successful_test("mean_threshold")
