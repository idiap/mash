# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-appserver isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR})

# List the source files of mash-appserver
set(SRCS interactive_application_server.cpp
         interactive_listener.cpp
)

add_library(mash-appserver STATIC ${SRCS})
set_target_properties(mash-appserver PROPERTIES COMPILE_FLAGS "-fPIC")
