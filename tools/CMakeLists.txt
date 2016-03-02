add_custom_target(tools-symlink-to-pymash ALL ln -f -n -s "${MASH_SOURCE_DIR}/pymash" pymash
                  WORKING_DIRECTORY "${MASH_SOURCE_DIR}/tools"
                  COMMENT "Creation of a symlink to the 'pymash' module for the tools")