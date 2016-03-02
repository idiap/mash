# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-network isn't part of the SDK, and isn't supported on Windows")
endif()

if (NOT MASH_ADVANCED_TESTS)
    message(FATAL_ERROR "Advanced tests aren't enabled")
endif()


function(add_classifier_crash_test METHOD_NAME)
    add_test("mash-experiment-server-classifier-crash-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "crash" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_crash_template.txt" "unittests/crash_in_${METHOD_NAME}" "predictor" "${METHOD_NAME}")
endfunction()


function(add_goalplanner_crash_test METHOD_NAME)
    add_test("mash-experiment-server-goalplanner-crash-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "crash" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanner_crash_template.txt" "unittests/crash_in_${METHOD_NAME}" "predictor" "${METHOD_NAME}")
endfunction()


function(add_classification_heuristic_crash_test METHOD_NAME)
    add_test("mash-experiment-server-classification-heuristic-crash-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "crash" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classification_heuristic_crash_template.txt" "unittests/crash_in_${METHOD_NAME}" "heuristic" "${METHOD_NAME}")
endfunction()


function(add_goalplanning_heuristic_crash_test METHOD_NAME)
    add_test("mash-experiment-server-goalplanning-heuristic-crash-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "crash" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanning_heuristic_crash_template.txt" "unittests/crash_in_${METHOD_NAME}" "heuristic" "${METHOD_NAME}")
endfunction()


function(add_classification_heuristic_budget_exhaustion_test METHOD_NAME)
    add_test("mash-experiment-server-classification-heuristic-budget-exhaustion-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "timeout" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classification_heuristic_timeout_template.txt" "unittests/time_budget_exhaustion_in_${METHOD_NAME}" "heuristic" "${METHOD_NAME}")
endfunction()


function(add_goalplanning_heuristic_budget_exhaustion_test METHOD_NAME)
    add_test("mash-experiment-server-goalplanning-heuristic-budget-exhaustion-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "timeout" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanning_heuristic_timeout_template.txt" "unittests/time_budget_exhaustion_in_${METHOD_NAME}" "heuristic" "${METHOD_NAME}")
endfunction()


function(add_classification_heuristic_timeout_test METHOD_NAME)
    add_test("mash-experiment-server-classification-heuristic-timeout-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "timeout" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classification_heuristic_timeout_template.txt" "unittests/timeout_in_${METHOD_NAME}" "heuristic" "${METHOD_NAME}")
endfunction()


function(add_goalplanning_heuristic_timeout_test METHOD_NAME)
    add_test("mash-experiment-server-goalplanning-heuristic-timeout-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "timeout" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanning_heuristic_timeout_template.txt" "unittests/timeout_in_${METHOD_NAME}" "heuristic" "${METHOD_NAME}")
endfunction()


function(add_classification_instrument_crash_test METHOD_NAME)
    add_test("mash-experiment-server-classification-instrument-crash-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "crash" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classification_instrument_crash_template.txt" "unittests/crash_in_${METHOD_NAME}" "instrument" "${METHOD_NAME}")
endfunction()

function(add_goalplanning_instrument_crash_test METHOD_NAME)
    add_test("mash-experiment-server-goalplanning-instrument-crash-in-${METHOD_NAME}" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_sandboxing.py" "crash" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanning_instrument_crash_template.txt" "unittests/crash_in_${METHOD_NAME}" "instrument" "${METHOD_NAME}")
endfunction()



# Create the classifier-related tests
add_classifier_crash_test("loading")
add_classifier_crash_test("constructor")
add_classifier_crash_test("setup")
add_classifier_crash_test("train")
add_classifier_crash_test("classify")
add_classifier_crash_test("reportfeaturesused")

# Create the goalplanner-related tests
add_goalplanner_crash_test("loading")
add_goalplanner_crash_test("constructor")
add_goalplanner_crash_test("setup")
add_goalplanner_crash_test("learn")
add_goalplanner_crash_test("chooseaction")
add_goalplanner_crash_test("reportfeaturesused")

# Create the heuristics-related tests
add_classification_heuristic_crash_test("loading")
add_classification_heuristic_crash_test("constructor")
add_classification_heuristic_crash_test("init")
add_classification_heuristic_crash_test("dim")
add_classification_heuristic_crash_test("prepareforsequence")
add_classification_heuristic_crash_test("prepareforimage")
add_classification_heuristic_crash_test("prepareforcoordinates")
add_classification_heuristic_crash_test("computefeature")
add_classification_heuristic_crash_test("finishforcoordinates")
add_classification_heuristic_crash_test("finishforimage")
add_classification_heuristic_crash_test("finishforsequence")

add_goalplanning_heuristic_crash_test("loading")
add_goalplanning_heuristic_crash_test("constructor")
add_goalplanning_heuristic_crash_test("init")
add_goalplanning_heuristic_crash_test("dim")
add_goalplanning_heuristic_crash_test("prepareforsequence")
add_goalplanning_heuristic_crash_test("prepareforimage")
add_goalplanning_heuristic_crash_test("prepareforcoordinates")
add_goalplanning_heuristic_crash_test("computefeature")
add_goalplanning_heuristic_crash_test("finishforcoordinates")
add_goalplanning_heuristic_crash_test("finishforimage")
add_goalplanning_heuristic_crash_test("finishforsequence")

add_classification_heuristic_budget_exhaustion_test("constructor")
add_classification_heuristic_budget_exhaustion_test("init")
add_classification_heuristic_budget_exhaustion_test("dim")
add_classification_heuristic_budget_exhaustion_test("prepareforsequence")
add_classification_heuristic_budget_exhaustion_test("prepareforimage")
add_classification_heuristic_budget_exhaustion_test("prepareforcoordinates")
add_classification_heuristic_budget_exhaustion_test("computefeature")
add_classification_heuristic_budget_exhaustion_test("finishforcoordinates")
add_classification_heuristic_budget_exhaustion_test("finishforimage")
add_classification_heuristic_budget_exhaustion_test("finishforsequence")

add_goalplanning_heuristic_budget_exhaustion_test("constructor")
add_goalplanning_heuristic_budget_exhaustion_test("init")
add_goalplanning_heuristic_budget_exhaustion_test("dim")
add_goalplanning_heuristic_budget_exhaustion_test("prepareforsequence")
add_goalplanning_heuristic_budget_exhaustion_test("prepareforimage")
add_goalplanning_heuristic_budget_exhaustion_test("prepareforcoordinates")
add_goalplanning_heuristic_budget_exhaustion_test("computefeature")
add_goalplanning_heuristic_budget_exhaustion_test("finishforcoordinates")
add_goalplanning_heuristic_budget_exhaustion_test("finishforimage")
add_goalplanning_heuristic_budget_exhaustion_test("finishforsequence")

add_classification_heuristic_timeout_test("constructor")
add_classification_heuristic_timeout_test("init")
add_classification_heuristic_timeout_test("dim")
add_classification_heuristic_timeout_test("prepareforsequence")
add_classification_heuristic_timeout_test("prepareforimage")
add_classification_heuristic_timeout_test("prepareforcoordinates")
add_classification_heuristic_timeout_test("computefeature")
add_classification_heuristic_timeout_test("finishforcoordinates")
add_classification_heuristic_timeout_test("finishforimage")
add_classification_heuristic_timeout_test("finishforsequence")

add_goalplanning_heuristic_timeout_test("constructor")
add_goalplanning_heuristic_timeout_test("init")
add_goalplanning_heuristic_timeout_test("dim")
add_goalplanning_heuristic_timeout_test("prepareforsequence")
add_goalplanning_heuristic_timeout_test("prepareforimage")
add_goalplanning_heuristic_timeout_test("prepareforcoordinates")
add_goalplanning_heuristic_timeout_test("computefeature")
add_goalplanning_heuristic_timeout_test("finishforcoordinates")
add_goalplanning_heuristic_timeout_test("finishforimage")
add_goalplanning_heuristic_timeout_test("finishforsequence")

# Create the instruments-related tests
add_classification_instrument_crash_test("loading")
add_classification_instrument_crash_test("constructor")
add_classification_instrument_crash_test("setup")
add_classification_instrument_crash_test("onclassificationexperimentstarted")
add_classification_instrument_crash_test("onclassifiertrainingstarted")
add_classification_instrument_crash_test("onclassifiertrainingdone")
add_classification_instrument_crash_test("onclassifierteststarted")
add_classification_instrument_crash_test("onclassifiertestdone")
add_classification_instrument_crash_test("onclassifierclassificationdone")
add_classification_instrument_crash_test("onfeaturelistreported")
add_classification_instrument_crash_test("onfeaturescomputedbyclassifier")
add_classification_instrument_crash_test("onexperimentdone")

add_goalplanning_instrument_crash_test("loading")
add_goalplanning_instrument_crash_test("constructor")
add_goalplanning_instrument_crash_test("setup")
add_goalplanning_instrument_crash_test("onplannerlearningstarted")
add_goalplanning_instrument_crash_test("onplannerlearningdone")
add_goalplanning_instrument_crash_test("onplannerteststarted")
add_goalplanning_instrument_crash_test("onplannertestdone")
add_goalplanning_instrument_crash_test("onplanneractionchoosen")
add_goalplanning_instrument_crash_test("onfeaturelistreported")
add_goalplanning_instrument_crash_test("onfeaturescomputedbyplanner")
add_goalplanning_instrument_crash_test("onexperimentdone")

add_test("mash-experiment-server-classification-instrument-report" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_instrument_report.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classification_instrument.txt" "unittests/methods_logger.data" "expected_classification_instrument.py")
add_test("mash-experiment-server-goalplanning-instrument-report" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_instrument_report.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanning_instrument.txt" "unittests/methods_logger.data" "expected_goalplanning_instrument.py")
add_test("mash-experiment-server-classifier-report" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_instrument_report.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_report.txt" "predictor.data" "expected_classifier_report.py")
add_test("mash-experiment-server-goalplanner-report" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_instrument_report.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanner_report.txt" "predictor.data" "expected_goalplanner_report.py")

# Create the model-related tests
add_test("mash-experiment-server-classification-model-saving" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_saving.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_saving.txt" "off" "expected_predictor.model")
add_test("mash-experiment-server-classification-model-saving-with-data" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_saving.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_saving_with_data.txt" "off" "expected_predictor.model" "expected_predictor.internal")
add_test("mash-experiment-server-classification-model-loading-command-line" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_loading.txt" "off" "on" "predictor.model")
add_test("mash-experiment-server-classification-model-loading-with-data-command-line" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_loading_with_data.txt" "off" "on" "predictor.model" "expected_predictor.internal")
add_test("mash-experiment-server-classification-model-loading-protocol" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_loading.txt" "off" "off" "predictor.model")
add_test("mash-experiment-server-classification-model-loading-with-data-protocol" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_loading_with_data.txt" "off" "off" "predictor.model" "expected_predictor.internal")
add_test("mash-experiment-server-classification-model-saving-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_saving.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_saving.txt" "on" "expected_predictor.model")
add_test("mash-experiment-server-classification-model-saving-with-data-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_saving.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_saving_with_data.txt" "on" "expected_predictor.model" "expected_predictor.internal")
add_test("mash-experiment-server-classification-model-loading-command-line-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_loading.txt" "on" "on" "predictor.model")
add_test("mash-experiment-server-classification-model-loading-with-data-command-line-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_loading_with_data.txt" "on" "on" "predictor.model" "expected_predictor.internal")
add_test("mash-experiment-server-classification-model-loading-protocol-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_loading.txt" "on" "on" "predictor.model")
add_test("mash-experiment-server-classification-model-loading-with-data-protocol-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classifier_model_loading_with_data.txt" "on" "off" "predictor.model" "expected_predictor.internal")

add_test("mash-experiment-server-goalplanning-model-saving" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_saving.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanner_model_saving.txt" "off" "expected_predictor.model")
add_test("mash-experiment-server-goalplanning-model-loading-command-line" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanner_model_loading.txt" "off" "on" "predictor.model")
add_test("mash-experiment-server-goalplanning-model-loading-protocol" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanner_model_loading.txt" "off" "off" "predictor.model")
add_test("mash-experiment-server-goalplanning-model-saving-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_saving.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanner_model_saving.txt" "on" "expected_predictor.model")
add_test("mash-experiment-server-goalplanning-model-loading-command-line-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanner_model_loading.txt" "on" "on" "predictor.model")
add_test("mash-experiment-server-goalplanning-model-loading-protocol-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_model_loading.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanner_model_loading.txt" "on" "on" "predictor.model")

# Create the notifications-related tests
add_test("mash-experiment-server-classification-notifications" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_notifications.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classification_notifications.txt" "off" "expected_classification_notifications.py")
add_test("mash-experiment-server-classification-notifications-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_notifications.py" "image" "${MASH_SOURCE_DIR}/application-servers/image-server/" "${MASH_BINARY_DIR}/bin" "settings_classification_notifications.txt" "on" "expected_classification_notifications.py")
add_test("mash-experiment-server-goalplanning-notifications" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_notifications.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanning_notifications.txt" "off" "expected_goalplanning_notifications.py")
add_test("mash-experiment-server-goalplanning-notifications-no-sandboxing" "${MASH_SOURCE_DIR}/tests/tests_experiment_server/test_notifications.py" "maze" "${MASH_BINARY_DIR}/bin" "${MASH_BINARY_DIR}/bin" "settings_goalplanning_notifications.txt" "on" "expected_goalplanning_notifications.py")
