#include <iostream>
#include "jchip8.h"
#include "sdl2_handler.h"

int main(int argc, char* argv[])
{
    sdl2_handler sdl_handler;
    JChip8 chip8;

    chip8.initialize();
    chip8.load_game("pong");

    std::uint32_t time = 0;
    while (time < 500)
    {
        time = sdl_handler.time();
        chip8.emulate_cycle();

        //if (chip8.draw_flag)
        //{
        //    draw_graphics();
        //}

        chip8.set_keys();
    }

    return 0;
}