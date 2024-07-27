project(${exe_name})

set(SOURCES
    "src/main.cpp"
    "src/jchip8.cpp"
    "src/sdl2_handler.cpp"
)

set(HEADERS
    "include/jchip8.h"
    "include/sdl2_handler.h"
    "include/typedefs.h"
    "include/emulator_config.h"
)

add_executable(JChip8 ${SOURCES} ${HEADERS})

# Set specific flags for each compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message("Setting flags for GNU/Clang compiler")
    target_compile_options(${exe_name} PRIVATE
        -std=c++2a
        -ggdb
        -g
        -Wall
        -Wextra
        -Wconversion
        -Wsign-conversion
        -pedantic-errors
        -fsanitize=address
        -fsanitize=undefined
    )
    target_link_options(${exe_name} PRIVATE
        -fsanitize=address
        -fsanitize=undefined
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("Setting flags for MSVC compiler")
    target_compile_options(${exe_name} PRIVATE
        /std:c++latest
        /W4
        /permissive-
        /sdl
        /Z7
        /Od
        /Wall
        /EHsc
    )
    target_link_options(${exe_name} PRIVATE)
endif()

target_include_directories(${exe_name} PRIVATE "include")

find_package(SDL2 REQUIRED)
target_link_libraries(${exe_name} PRIVATE SDL2::SDL2 SDL2::SDL2main)

add_custom_command(TARGET ${exe_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suite_roms
    ${CMAKE_CURRENT_BINARY_DIR}/test_suite_roms
)

#add_custom_command(TARGET ${exe_name} PRE_BUILD
    #COMMAND ${CMAKE_COMMAND} -E copy_directory
    #${CMAKE_CURRENT_SOURCE_DIR}/roms
    #${CMAKE_CURRENT_BINARY_DIR}/roms
#)
