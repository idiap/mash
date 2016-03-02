# List the source files
set(SRCS tinystr.cpp
         tinyxml.cpp
         tinyxmlerror.cpp
         tinyxmlparser.cpp
)

# Declaration of the library
add_library(tinyxml STATIC ${SRCS})

set_target_properties(tinyxml PROPERTIES COMPILE_FLAGS "-w -fPIC")
