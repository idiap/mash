add_subdirectory(unittests_mashutils)
add_subdirectory(unittests_mashcore)

if (NOT WIN32)
    add_subdirectory(unittests_mashnetwork)
    add_subdirectory(unittests_mashclassification)
    add_subdirectory(unittests_mashgoalplanning)
    add_subdirectory(unittests_mashinstrumentation)
endif()
