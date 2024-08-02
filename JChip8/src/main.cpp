#include "emulator_config.h"
#include "imgui_handler.h"
#include "jchip8.h"
#include "sdl2_handler.h"
#include "typedefs.h"
#include "j_assembler.h"

static constexpr uint32 WINDOW_WIDTH  = 640;
static constexpr uint32 WINDOW_HEIGHT = 320;

int main(int argc, char* argv[])
{
    emulator_config config = load_configuration_file();

    sdl2_handler sdl_handler{ WINDOW_WIDTH, WINDOW_HEIGHT, config };
    imgui_handler gui{ sdl_handler };
    JChip8 chip8{ 1000 };

    uint16 menu_height = gui.get_window_height();
    sdl_handler.set_window_size(WINDOW_WIDTH, WINDOW_HEIGHT + menu_height);

    while (chip8.state != emulator_state::quit)
    {
        sdl_handler.clear_framebuffer();
        sdl_handler.handle_input(chip8, gui);

        if (chip8.state == emulator_state::paused)
            continue;

        uint64 before_frame = sdl_handler.time();

        if (chip8.rom_loaded())
        {
            for (uint16 i = 0; i < chip8.ips / 60; ++i)
            {
                chip8.emulate_cycle();

                if ((chip8.current_instruction().opcode >> 12) == DRAW_INSTRUCTION)
                    break;
            }
            uint64 after_frame = sdl_handler.time();
            const double time_elapsed = static_cast<double>((after_frame - before_frame) / 1000) / sdl_handler.performance_freq();
            sdl_handler.delay(16.67f > time_elapsed ? 16.67f - time_elapsed : 0);
            sdl_handler.draw_graphics(chip8);

            chip8.update_timers(sdl_handler);
        }

        gui.begin_frame(sdl_handler);
        gui.draw_gui(chip8);

        if (gui.init_default_config())
            create_default_config_file();
        if (gui.reload_config())
            config = load_configuration_file();

        gui.end_frame();

        SDL_RenderPresent(sdl_handler.renderer());
    }

    return 0;
}