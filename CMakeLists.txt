cmake_minimum_required (VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message("Setting flags for GNU/Clang compiler")
    add_compile_options(
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
    add_link_options(
        -fsanitize=address
        -fsanitize=undefined
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("Setting flags for MSVC compiler")
    add_compile_options(
        /std:c++latest
        /W4
        /permissive-
        /sdl
        /Z7
        /Od
        /Wall
        /EHsc
    )
    add_link_options()
endif()

set(assembler_name JChip8Asm)
set(exe_name JChip8)
add_subdirectory(${assembler_name})
add_subdirectory(${exe_name})
