add_custom_target(image-server__symlink-to-pymash ALL ln -f -n -s "${MASH_SOURCE_DIR}/pymash" pymash
                  WORKING_DIRECTORY "${MASH_SOURCE_DIR}/application-servers/image-server"
                  COMMENT "Creation of a symlink to the 'pymash' module for 'image-server'")
