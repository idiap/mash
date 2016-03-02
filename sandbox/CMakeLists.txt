# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The program 'sandbox' isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}
                    ${MASH_SOURCE_DIR}/dependencies
                    ${MASH_SOURCE_DIR}/dependencies/include
                    ${MASH_SOURCE_DIR}/dependencies/FreeImage)

# List the source files of sandbox-warden
set(SRCS warden.c)

if (NOT APPLE)
    # Create and link the warden library
    add_library(sandbox-warden SHARED ${SRCS})

    target_link_libraries(sandbox-warden dl)

    set_target_properties(sandbox-warden PROPERTIES INSTALL_RPATH "."
                                                    BUILD_WITH_INSTALL_RPATH ON
                                                    COMPILE_FLAGS "-fPIC")

    if (MASH_SANDBOX_MEMORY_DEBUG)
        set_target_properties(sandbox-warden PROPERTIES COMPILE_FLAGS "-fPIC -DMASH_WARDEN_MEMORY_DEBUG")
    else()
        set_target_properties(sandbox-warden PROPERTIES COMPILE_FLAGS "-fPIC")
    endif()
endif()

# List the source files of sandbox
set(SRCS main.cpp
         sandbox.cpp
         sandboxed_object.cpp
         sandboxed_classifier.cpp
         sandboxed_heuristics.cpp
         sandboxed_planner.cpp
         sandboxed_instruments.cpp
         sandbox_input_set.cpp
         sandbox_notifier.cpp
         sandbox_perception.cpp
         sandbox_task.cpp
)

# Create and link the executable
add_executable(sandbox ${SRCS})

if (NOT APPLE)
    add_dependencies(sandbox sandbox-warden)
    target_link_libraries(sandbox sandbox-warden)
endif()

add_dependencies(sandbox mash-classification mash-goalplanning
                         mash-instrumentation mash-core mash-sandboxing
                         mash-utils freeimage)

target_link_libraries(sandbox mash-classification mash-goalplanning
                              mash-instrumentation mash-core mash-sandboxing
                              mash-utils freeimage)

set_target_properties(sandbox PROPERTIES INSTALL_RPATH "."
                                         BUILD_WITH_INSTALL_RPATH ON
                                         COMPILE_FLAGS "-fPIC")


# List the source files of coredump_rights_changer
set(SRCS coredump_rights_changer.cpp)

# Create and link the executable
add_executable(coredump_rights_changer ${SRCS})

set_target_properties(coredump_rights_changer PROPERTIES INSTALL_RPATH "."
                                                         BUILD_WITH_INSTALL_RPATH ON
                                                         COMPILE_FLAGS "-fPIC")
