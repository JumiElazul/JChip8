#include "emulator_config.h"
#include "jchip8.h"
#include "sdl2_handler.h"
#include "typedefs.h"
#include "j_assembler.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

int main(int argc, char* argv[])
{
    emulator_config config;
    sdl2_handler sdl_handler{ config };
    JChip8 chip8{ 1000 };

    chip8.load_next_test_rom();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(sdl_handler.window(), sdl_handler.renderer());
    ImGui_ImplSDLRenderer2_Init(sdl_handler.renderer());

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

        chip8.update_timers(sdl_handler);

        ImGui_ImplSDL2_NewFrame(sdl_handler.window());
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Game"))
            {
                if (ImGui::MenuItem("Load ROM"))
                {
                    // Handle ROM loading here
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Configuration"))
            {
                if (ImGui::MenuItem("Settings"))
                {
                    // Handle settings here
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::Render();
        sdl_handler.clear_framebuffer();
        // Draw Chip8 graphics (if needed)
        sdl_handler.draw_graphics(chip8);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(sdl_handler.renderer());
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return 0;
}