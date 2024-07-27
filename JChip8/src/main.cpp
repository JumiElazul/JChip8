#include "jchip8.h"
#include "sdl2_handler.h"
#include "typedefs.h"
#include <array>
#include <filesystem>
#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

std::vector<ROM> load_test_suite_roms()
{
    std::vector<ROM> roms;
    roms.reserve(8);
    std::filesystem::path path = std::filesystem::current_path().append("test_suite_roms");

    auto get_rom_size = [&](const std::string& filepath) {
        std::ifstream rom_file(filepath, std::ios::binary);
        if (!rom_file) throw std::runtime_error("Could not open file");

        rom_file.seekg(0, std::ios::end);
        uint16 size = static_cast<uint16>(rom_file.tellg());
        return size;
    };

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".ch8")
            {
                ROM rom =
                {
                    .filepath = entry.path().string(),
                    .name = entry.path().filename().replace_extension("").string(),
                    .size = get_rom_size(rom.filepath)
                };

                roms.push_back(rom);
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception swallower: " << e.what() << '\n';
    }

    return roms;
}

int main(int argc, char* argv[])
{
    sdl2_handler sdl_handler;
    JChip8 chip8{ 100 };
    int16 frame_wait_time = 1000 / chip8.ips;
    std::vector<ROM> roms = load_test_suite_roms();

    int rom_index = 0;
    chip8.load_game(roms[rom_index]);

    while (chip8.state != emulator_state::quit) 
    {
        int current_rom_index = rom_index;

        int32_t frame_start_time = sdl_handler.time();
        sdl_handler.handle_input(chip8, &rom_index);
        if (chip8.state == emulator_state::paused)
            continue;

        if (rom_index != current_rom_index)
            chip8.load_game(roms[rom_index]);

        chip8.emulate_cycle();

        if (chip8.draw_flag())
        {
            sdl_handler.draw_graphics(chip8);
            chip8.reset_draw_flag();
        }

        int32_t frame_end_time = sdl_handler.time();
        int32_t frame_duration = frame_end_time - frame_start_time;
        if (frame_duration < frame_wait_time)
            std::this_thread::sleep_for(std::chrono::milliseconds(frame_wait_time - frame_duration));
    }

    return 0;
}