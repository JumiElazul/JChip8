#include "jchip8.h"
#include "sdl2_handler.h"
#include "typedefs.h"
#include <thread>

int main(int argc, char* argv[])
{
    sdl2_handler sdl_handler;
    JChip8* chip8 = new JChip8{ 5 };
    int16 frame_wait_time = 1000 / chip8->ips;

    chip8->load_game("roms/IBMLogo.ch8");

    int32_t last_frame_time = sdl_handler.time();
    while (chip8->state != emulator_state::quit) 
    {
        int32_t frame_start_time = sdl_handler.time();




        if (chip8->state == emulator_state::paused)
            continue;

        sdl_handler.handle_input(*chip8);
        chip8->emulate_cycle();

        if (chip8->draw_flag())
        {
            bool* gfx = chip8->graphics;
            sdl_handler.draw_graphics(gfx, 64 * 32);
        }



        int32_t frame_end_time = sdl_handler.time();
        int32_t frame_duration = frame_end_time - frame_start_time;
        if (frame_duration < frame_wait_time)
            std::this_thread::sleep_for(std::chrono::milliseconds(frame_wait_time - frame_duration));
    }

    delete chip8;
    return 0;
}