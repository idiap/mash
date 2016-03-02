# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The tests aren't part of the SDK, and aren't supported on Windows")
endif()

add_subdirectory(tests_application_servers)
add_subdirectory(tests_mashsandboxing)
add_subdirectory(tests_mashnetwork)

if (MASH_ADVANCED_TESTS)

    if (NOT APPLE)
        add_subdirectory(tests_sandbox)
    endif()
    
    add_subdirectory(tests_mashcore)
    add_subdirectory(tests_mashclassification)
    add_subdirectory(tests_mashgoalplanning)
    add_subdirectory(tests_mashinstrumentation)
    add_subdirectory(tests_compilation_server)
    add_subdirectory(tests_experiment_server)
endif()
