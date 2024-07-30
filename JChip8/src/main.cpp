#include "emulator_config.h"
#include "jchip8.h"
#include "sdl2_handler.h"
#include "typedefs.h"
#include "j_assembler.h"

int main(int argc, char* argv[])
{
    emulator_config config;
    sdl2_handler sdl_handler{ config };
    JChip8 chip8{ 1000 };

    chip8.load_next_test_rom();

    while (chip8.state != emulator_state::quit) 
    {
        sdl_handler.handle_input(chip8);

        if (chip8.state == emulator_state::paused)
            continue;

        uint64 before_frame = sdl_handler.time();

        for (uint16 i = 0; i < chip8.ips / 60; ++i)
        {
            chip8.emulate_cycle();

            if ((chip8.current_instruction().opcode >> 12) == DRAW_INSTRUCTION)
                break;
        }
        uint64 after_frame = sdl_handler.time();
        const double time_elapsed = static_cast<double>((after_frame - before_frame) / 1000) / sdl_handler.performance_freq();
        sdl_handler.delay(16.67f > time_elapsed ? 16.67f - time_elapsed : 0);

        if (chip8.draw_flag())
        {
            sdl_handler.draw_graphics(chip8);
            chip8.reset_draw_flag();
        }

        chip8.update_timers();
    }

    return 0;
}