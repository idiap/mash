# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-goalplanning isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR})

# List the source files of mash-goalplanning
set(SRCS perception.cpp
         planners_manager.cpp
         sandboxed_planner.cpp
         trusted_planner.cpp
         sandbox_task_proxy.cpp
         task.cpp
         task_controller.cpp
)

add_library(mash-goalplanning SHARED ${SRCS})

add_dependencies(mash-goalplanning mash-core mash-network mash-utils)
target_link_libraries(mash-goalplanning mash-core mash-network mash-utils dl)

set_target_properties(mash-goalplanning PROPERTIES INSTALL_RPATH ".")
set_target_properties(mash-goalplanning PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
set_target_properties(mash-goalplanning PROPERTIES COMPILE_FLAGS "-fPIC")


# Installation stuff - Experiments Server
install(TARGETS mash-goalplanning
        LIBRARY DESTINATION experiment-server
		CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )

install(FILES planner.h perception_interface.h task_interface.h declarations.h
        DESTINATION experiment-server/mash-goalplanning
        CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )
