# Safeguard
if (MASH_SDK OR WIN32)
    message(FATAL_ERROR "The library mash-instrumentation isn't part of the SDK, and isn't supported on Windows")
endif()


# Setup the search paths
include_directories(${MASH_SOURCE_DIR}
                    ${MASH_SOURCE_DIR}/dependencies
                    ${MASH_SOURCE_DIR}/dependencies/include
                    ${MASH_SOURCE_DIR}/dependencies/FreeImage)

# List the source files of mash-classification
set(SRCS instruments_manager.cpp
         sandboxed_instruments_set.cpp
         trusted_instruments_set.cpp
)

add_library(mash-instrumentation SHARED ${SRCS})

add_dependencies(mash-instrumentation mash-goalplanning mash-classification mash-core mash-network mash-sandboxing mash-utils)
target_link_libraries(mash-instrumentation mash-goalplanning mash-classification mash-core mash-network mash-sandboxing mash-utils dl)

set_target_properties(mash-instrumentation PROPERTIES INSTALL_RPATH ".")
set_target_properties(mash-instrumentation PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
set_target_properties(mash-instrumentation PROPERTIES COMPILE_FLAGS "-fPIC")


# Installation stuff - Experiments Server
install(TARGETS mash-instrumentation
        LIBRARY DESTINATION experiment-server
		CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )

install(FILES data_writer.h
        DESTINATION experiment-server/mash-instrumentation
        CONFIGURATIONS Release
        COMPONENT "experiment-server"
       )
