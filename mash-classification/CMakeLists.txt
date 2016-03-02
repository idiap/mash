# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-classification isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}
                    ${MASH_SOURCE_DIR}/dependencies
                    ${MASH_SOURCE_DIR}/dependencies/include
                    ${MASH_SOURCE_DIR}/dependencies/FreeImage)

# List the source files of mash-classification
set(SRCS classifier_input_set.cpp
         sandboxed_classifier.cpp
         trusted_classifier
         classifiers_manager.cpp
         sandbox_input_set_proxy.cpp
         dataset.cpp
         image_database.cpp
         stepper.cpp
)

add_library(mash-classification SHARED ${SRCS})

add_dependencies(mash-classification mash-core mash-network mash-sandboxing mash-utils)
target_link_libraries(mash-classification mash-core mash-network mash-sandboxing mash-utils dl)

set_target_properties(mash-classification PROPERTIES INSTALL_RPATH ".")
set_target_properties(mash-classification PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
set_target_properties(mash-classification PROPERTIES COMPILE_FLAGS "-fPIC")


# Installation stuff - Experiments Server
install(TARGETS mash-classification
        LIBRARY DESTINATION experiment-server
		CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )

install(FILES classifier.h classifier_input_set_interface.h declarations.h
        DESTINATION experiment-server/mash-classification
        CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )
