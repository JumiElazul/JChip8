#include <iostream>
#include "jchip8.h"
#include "sdl2_handler.h"

int main(int argc, char* argv[])
{
    sdl2_handler sdl_handler(10.0f);
    JChip8 chip8;

    chip8.load_game("roms/IBMLogo.ch8");

    std::uint32_t time = 0;
    while (chip8.state != emulator_state::quit)
    {
        if (chip8.state == emulator_state::paused)
            continue;

        sdl_handler.handle_input(chip8);
        time = sdl_handler.time();
        chip8.emulate_cycle();

        //if (chip8.draw_flag)
        //{
        //    unsigned char* gfx = chip8.graphics;
        //    sdl_handler.draw_graphics(gfx, GRAPHICS_WIDTH * GRAPHICS_HEIGHT);
        //}
    }

    return 0;
}