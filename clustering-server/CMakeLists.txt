# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The Compilation Server isn't part of the SDK, and isn't supported on Windows")
endif()


# Symlink the pymash module
add_custom_target(clustering-server-symlink-to-pymash ALL ln -f -n -s "${MASH_SOURCE_DIR}/pymash" pymash
                  WORKING_DIRECTORY "${MASH_SOURCE_DIR}/clustering-server"
                  COMMENT "Creation of a symlink to the 'pymash' module for 'clustering-server'")
