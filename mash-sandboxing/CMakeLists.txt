# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-sandboxing isn't part of the SDK, and isn't supported on Windows")
endif()

# Setup the search paths
include_directories(${MASH_SOURCE_DIR})

# List the source files of mash-sandboxing
set(SRCS communication_channel.cpp
         sandbox_controller.cpp
)

# Create the library
add_library(mash-sandboxing SHARED ${SRCS})
add_dependencies(mash-sandboxing mash-utils)
target_link_libraries(mash-sandboxing mash-utils dl)

set_target_properties(mash-sandboxing PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(mash-sandboxing PROPERTIES INSTALL_RPATH ".")
set_target_properties(mash-sandboxing PROPERTIES BUILD_WITH_INSTALL_RPATH ON)


# Installation stuff - Compilation Server
install(TARGETS mash-sandboxing
        LIBRARY DESTINATION compilation-server
		CONFIGURATIONS Release
        COMPONENT "compilation-server"
)

# Installation stuff - Experiments Server
install(TARGETS mash-sandboxing
        LIBRARY DESTINATION experiment-server
		CONFIGURATIONS Release
        COMPONENT "experiment-server"
)
