# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The Compilation Server isn't part of the SDK, and isn't supported on Windows")
endif()


# Symlink the pymash module
add_custom_target(compilation-server-symlink-to-pymash ALL ln -f -n -s "${MASH_SOURCE_DIR}/pymash" pymash
                  WORKING_DIRECTORY "${MASH_SOURCE_DIR}/compilation-server"
                  COMMENT "Creation of a symlink to the 'pymash' module for 'compilation-server'")

# Setup the search paths
include_directories(${MASH_SOURCE_DIR}
                    ${MASH_SOURCE_DIR}/dependencies
                    ${MASH_SOURCE_DIR}/dependencies/include
                    ${MASH_SOURCE_DIR}/dependencies/FreeImage)

# Create and link the executable
add_executable(checkheuristic checkheuristic.cpp)
add_dependencies(checkheuristic mash-core freeimage)

target_link_libraries(checkheuristic mash-core)

set_target_properties(checkheuristic PROPERTIES INSTALL_RPATH "."
                                                BUILD_WITH_INSTALL_RPATH ON
                                                COMPILE_FLAGS "-fPIC")

if (MASH_USE_CURL AND CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(checkheuristic ${CURL_LIBRARIES})
    set_target_properties(checkheuristic PROPERTIES COMPILE_DEFINITIONS "USE_CURL")
endif()


# Installation stuff
install(TARGETS checkheuristic
        RUNTIME DESTINATION compilation-server
      CONFIGURATIONS Release
        COMPONENT "compilation-server"
       )

install(PROGRAMS compilation-server.py config_template.py
        DESTINATION compilation-server
        CONFIGURATIONS Release
        COMPONENT "compilation-server"
       )

install(DIRECTORY heuristics_cmake
        DESTINATION compilation-server
        CONFIGURATIONS Release
        COMPONENT "compilation-server"
        PATTERN CMakeDeclarations.* EXCLUDE
       )

install(FILES heuristics_cmake/CMakeDeclarations.install
        DESTINATION compilation-server/heuristics_cmake
        CONFIGURATIONS Release
        COMPONENT "compilation-server"
        RENAME "CMakeDeclarations.txt"
       )
