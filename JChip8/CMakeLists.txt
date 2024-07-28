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

add_executable(${exe_name} ${SOURCES} ${HEADERS})

target_include_directories(${exe_name} PRIVATE "include")

find_package(SDL2 REQUIRED)
target_link_libraries(${exe_name} PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(${exe_name} PRIVATE ${assembler_name})

add_custom_command(TARGET ${exe_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suite_roms
    ${CMAKE_CURRENT_BINARY_DIR}/test_suite_roms
)
