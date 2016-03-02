# Setup the search paths
include_directories(${MASH_SOURCE_DIR}
                    ${MASH_SOURCE_DIR}/dependencies
                    ${MASH_SOURCE_DIR}/dependencies/include)

# List the source files of maze-server
set(SRCS main.cpp
         maze_server.cpp
         world.cpp
)

# Create and link the executable
add_executable(maze-server ${SRCS})
add_dependencies(maze-server mash-network mash-appserver)

target_link_libraries(maze-server mash-network mash-appserver)

set_target_properties(maze-server PROPERTIES INSTALL_RPATH "."
                                             BUILD_WITH_INSTALL_RPATH ON
                                             COMPILE_FLAGS "-fPIC")


# Installation stuff
install(TARGETS maze-server
        RUNTIME DESTINATION maze-server
		CONFIGURATIONS Release
        COMPONENT "maze-server"
       )
