# Safeguard
if (MASH_SDK OR WIN32 OR APPLE)
    message(FATAL_ERROR "The library sandbox-warden isn't part of the SDK, and isn't supported on Windows and MacOS X")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}/dependencies ${MASH_SOURCE_DIR} ${MASH_SOURCE_DIR}/tests)

# List the source files (each one define a test)
set(TESTS_SRCS testWarden_MemoryWatching_malloc.cpp
               testWarden_MemoryWatching_calloc.cpp
               testWarden_MemoryWatching_realloc.cpp
               testWarden_MemoryWatching_valloc.cpp
               testWarden_MemoryWatching_nolimit.cpp
               testWarden_MemoryWatching_new.cpp
               testWarden_ForbiddenSystemCall_system.cpp
               testWarden_ForbiddenSystemCall_popen.cpp
               testWarden_ForbiddenSystemCall_pclose.cpp
               testWarden_ForbiddenSystemCall_fdopen.cpp
               testWarden_ForbiddenSystemCall_fopen.cpp
               testWarden_ForbiddenSystemCall_freopen.cpp
               testWarden_ForbiddenSystemCall_fclose.cpp
               testWarden_ForbiddenSystemCall_tmpfile.cpp
               testWarden_ForbiddenSystemCall_dlopen.cpp
               testWarden_ForbiddenSystemCall_dlclose.cpp
               testWarden_ForbiddenSystemCall_socket.cpp
               testWarden_ForbiddenSystemCall_close.cpp
               testWarden_ForbiddenSystemCall_execve.cpp
               testWarden_ForbiddenSystemCall_execl.cpp
               testWarden_ForbiddenSystemCall_execle.cpp
               testWarden_ForbiddenSystemCall_execlp.cpp
               testWarden_ForbiddenSystemCall_execv.cpp
               testWarden_ForbiddenSystemCall_execvp.cpp
               testWarden_ForbiddenSystemCall_exit.cpp
               testWarden_ForbiddenSystemCall__exit.cpp
               testWarden_ForbiddenSystemCall__exit2.cpp
               testWarden_ForbiddenSystemCall_atexit.cpp
               testWarden_ForbiddenSystemCall_wait.cpp
               testWarden_ForbiddenSystemCall_wait3.cpp
               testWarden_ForbiddenSystemCall_wait4.cpp
               testWarden_ForbiddenSystemCall_waitpid.cpp
               testWarden_ForbiddenSystemCall_sleep.cpp
               testWarden_ForbiddenSystemCall_usleep.cpp
               testWarden_ForbiddenSystemCall_nanosleep.cpp
               testWarden_ForbiddenSystemCall_fork.cpp
               testWarden_ForbiddenSystemCall_vfork.cpp
               testWarden_ForbiddenSystemCall_signal.cpp
               testWarden_ForbiddenSystemCall_sigaction.cpp
               testWarden_ForbiddenSystemCall_sigaltstack.cpp
               testWarden_ForbiddenSystemCall_sigprocmask.cpp
               testWarden_ForbiddenSystemCall_sigsuspend.cpp
               testWarden_ForbiddenSystemCall_siginterrupt.cpp
               testWarden_ForbiddenSystemCall_kill.cpp
               testWarden_ForbiddenSystemCall_longjmp.cpp
               testWarden_ForbiddenSystemCall_setjmp.cpp
               testWarden_ForbiddenSystemCall_siglongjmp.cpp
               testWarden_ForbiddenSystemCall_sigsetjmp.cpp
)


# Create a target for each test
foreach (TEST_SRC ${TESTS_SRCS})

	string(REPLACE ".cpp" "" TEST ${TEST_SRC})

    # Create the target
	add_executable(${TEST} ${TEST_SRC})
	add_dependencies(${TEST} sandbox-warden)

	target_link_libraries(${TEST} sandbox-warden dl)

    get_target_property(OUTPUT_DIRECTORY ${TEST} RUNTIME_OUTPUT_DIRECTORY)

    set_target_properties(${TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/tests_sandbox"
                                             INSTALL_RPATH "."
                                             BUILD_WITH_INSTALL_RPATH ON
                                             COMPILE_FLAGS "-fPIC")

    # Create the test
    add_test(${TEST} "bash" "-c" "${MASH_SOURCE_DIR}/tests/tests_sandbox/run.sh \"${OUTPUT_DIRECTORY}\" \"${OUTPUT_DIRECTORY}/libsandbox-warden.so\" \"./tests_sandbox/${TEST}\"")
    
endforeach()
