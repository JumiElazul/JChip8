#include "jchip8.h"
#include "sdl2_handler.h"
#include "typedefs.h"
#include <filesystem>
#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>

const char* rom0 = "IBMLogo";
const char* rom1 = "Blinky [Hans Christian Egeberg, 1991]";

std::unordered_map<std::string, ROM> load_default_roms()
{
    std::unordered_map<std::string, ROM> roms;
    std::filesystem::path path = std::filesystem::current_path().append("roms");

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".ch8")
            {
                ROM rom =
                {
                    .filepath = entry.path().string(),
                    .name = entry.path().filename().replace_extension("").string()
                };

                std::ifstream rom_file(entry.path().string(), std::ios::binary);
                if (!rom_file) throw std::runtime_error("Could not open file");

                rom_file.seekg(0, std::ios::end);
                uint16 size = static_cast<uint16>(rom_file.tellg());
                rom.size = size;

                roms[rom.name] = rom;
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
    JChip8 chip8{ 8 };
    int16 frame_wait_time = 1000 / chip8.ips;
    std::unordered_map<std::string, ROM> roms = load_default_roms();

    chip8.load_game(roms[rom0]);

    while (chip8.state != emulator_state::quit) 
    {
        int32_t frame_start_time = sdl_handler.time();


        if (chip8.state == emulator_state::paused)
            continue;

        sdl_handler.handle_input(chip8);
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