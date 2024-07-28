project(${assembler_name})

set(SOURCES
    "src/j_assembler.cpp"
)

set(HEADERS
    "include/j_assembler.h"
)

add_library(${assembler_name} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${assembler_name} PUBLIC "include")
