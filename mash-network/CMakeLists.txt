# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-network isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR})

# List the source files of mash-network
set(SRCS busy_listener.cpp
         client.cpp
         commands_serializer.cpp
         network_buffer.cpp
         networkutils.cpp
         server.cpp
         server_listener.cpp
)

add_library(mash-network SHARED ${SRCS})
add_dependencies(mash-network mash-utils)
target_link_libraries(mash-network mash-utils)

set_target_properties(mash-network PROPERTIES INSTALL_RPATH ".")
set_target_properties(mash-network PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
set_target_properties(mash-network PROPERTIES COMPILE_FLAGS "-fPIC")


# Installation stuff - Compilation Server
install(TARGETS mash-network
        LIBRARY DESTINATION compilation-server
		CONFIGURATIONS Release
        COMPONENT "compilation-server"
       )


# Installation stuff - Experiments Server
install(TARGETS mash-network
        LIBRARY DESTINATION experiment-server
		CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )
