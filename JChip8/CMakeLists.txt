project(${exe_name})

set(SOURCES
    "src/main.cpp"
    "src/emulator_config.cpp"
    "src/jchip8.cpp"
    "src/imgui_handler.cpp"
    "src/sdl2_handler.cpp"
)

set(HEADERS
    "include/emulator_config.h"
    "include/jchip8.h"
    "include/imgui_handler.h"
    "include/sdl2_handler.h"
    "include/typedefs.h"
)

add_executable(${exe_name} ${SOURCES} ${HEADERS})

target_include_directories(${exe_name} PRIVATE "include")

find_package(SDL2 REQUIRED)
find_package(IMGUI REQUIRED)
find_package(tinyfiledialogs REQUIRED)
target_link_libraries(${exe_name} PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(${exe_name} PRIVATE imgui::imgui)
target_link_libraries(${exe_name} PRIVATE tinyfiledialogs::tinyfiledialogs)
target_link_libraries(${exe_name} PRIVATE ${assembler_name})

add_custom_command(TARGET ${exe_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suite_roms
    ${CMAKE_CURRENT_BINARY_DIR}/test_suite_roms
)
