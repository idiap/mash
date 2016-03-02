# List the source files of UnitTest++
file(GLOB SRCS src/*.cpp)

if (WIN32)
    file(GLOB SRCS_PLATFORM src/Win32/*.cpp)
else ()
    file(GLOB SRCS_PLATFORM src/Posix/*.cpp)
endif ()

list(APPEND SRCS ${SRCS_PLATFORM})

add_library(UnitTest++ STATIC ${SRCS})

if (NOT WIN32)
    set_target_properties(UnitTest++ PROPERTIES COMPILE_FLAGS "-fPIC")
endif()
