# Setup the search paths
include_directories(.
                    ${MASH_SOURCE_DIR}
                    ${MASH_SOURCE_DIR}/dependencies
                    ${MASH_SOURCE_DIR}/dependencies/include)

# List the source files of maze-server
set(SRCS main.cpp
         goalplanning_simulator.cpp
         logics/logic_2d_movements.cpp
         logics/logic_recorded.cpp
)

# Create and link the executable
add_executable(goalplanning-simulator ${SRCS})
add_dependencies(goalplanning-simulator mash-network mash-appserver)

target_link_libraries(goalplanning-simulator mash-network mash-appserver)

set_target_properties(goalplanning-simulator PROPERTIES INSTALL_RPATH "."
                                                        BUILD_WITH_INSTALL_RPATH ON
                                                        COMPILE_FLAGS "-fPIC")


# Installation stuff
install(TARGETS goalplanning-simulator
        RUNTIME DESTINATION goalplanning-simulator
		CONFIGURATIONS Release
        COMPONENT "goalplanning-simulator"
       )
