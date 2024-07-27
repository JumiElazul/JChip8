#include "sdl2_handler.h"
#include "emulator_config.h"
#include "JChip8.h"
#include "typedefs.h"
#include <SDL2/SDL.h>
#include <iostream>

sdl2_handler::sdl2_handler(const emulator_config& config)
    : _window(nullptr)
    , _renderer(nullptr)
    , _config(config)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    int32 window_width = 640;
    int32 window_height = 320;

    _window = SDL_CreateWindow("JChip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN);
    if (!_window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    if (!_renderer)
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }
}

sdl2_handler::~sdl2_handler()
{
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

uint32 sdl2_handler::time() const noexcept
{
    return SDL_GetTicks();
}

SDL_Window* sdl2_handler::window() const noexcept { return _window; }
SDL_Renderer* sdl2_handler::renderer() const noexcept { return _renderer; }

void sdl2_handler::draw_graphics(JChip8& chip8)
{
    bool* gfx = &chip8.graphics[0];
    float scale_x = 640.0f / GRAPHICS_WIDTH;
    float scale_y = 320.0f / GRAPHICS_HEIGHT;

    uint8 bg_r;
    uint8 bg_g;
    uint8 bg_b;
    uint8 bg_a;
    uint8 fg_r;
    uint8 fg_g;
    uint8 fg_b;
    uint8 fg_a;
    extract_rgba(_config.bg_color, bg_r, bg_g, bg_b, bg_a);
    extract_rgba(_config.fg_color, fg_r, fg_g, fg_b, fg_a);

    uint16 gfx_size = GRAPHICS_WIDTH * GRAPHICS_HEIGHT;
    SDL_Rect pixel;

    for (uint16 i = 0; i < gfx_size; ++i)
    {
        if (gfx[i]) SDL_SetRenderDrawColor(_renderer, fg_r, fg_g, fg_b, fg_a);
        else        SDL_SetRenderDrawColor(_renderer, bg_r, bg_g, bg_b, bg_a);

        int32 x = i % GRAPHICS_WIDTH;
        int32 y = i / GRAPHICS_WIDTH;
        pixel = { static_cast<int>(x * scale_x), static_cast<int>(y * scale_y), static_cast<int>(scale_x), static_cast<int>(scale_y) };

        SDL_RenderFillRect(_renderer, &pixel);

        if (_config.pixel_outlines)
        {
            SDL_SetRenderDrawColor(_renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderDrawRect(_renderer, &pixel);
        }
    }
    SDL_RenderPresent(_renderer);
}

void sdl2_handler::handle_input(JChip8& chip8)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            {
                chip8.state = emulator_state::quit;
                break;
            }
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_1: chip8.keypad[0x1] = true; break;
                    case SDL_SCANCODE_2: chip8.keypad[0x2] = true; break;
                    case SDL_SCANCODE_3: chip8.keypad[0x3] = true; break;
                    case SDL_SCANCODE_4: chip8.keypad[0xC] = true; break;
                    case SDL_SCANCODE_Q: chip8.keypad[0x4] = true; break;
                    case SDL_SCANCODE_W: chip8.keypad[0x5] = true; break;
                    case SDL_SCANCODE_E: chip8.keypad[0x6] = true; break;
                    case SDL_SCANCODE_R: chip8.keypad[0xD] = true; break;
                    case SDL_SCANCODE_A: chip8.keypad[0x7] = true; break;
                    case SDL_SCANCODE_S: chip8.keypad[0x8] = true; break;
                    case SDL_SCANCODE_D: chip8.keypad[0x9] = true; break;
                    case SDL_SCANCODE_F: chip8.keypad[0xE] = true; break;
                    case SDL_SCANCODE_Z: chip8.keypad[0xA] = true; break;
                    case SDL_SCANCODE_X: chip8.keypad[0x0] = true; break;
                    case SDL_SCANCODE_C: chip8.keypad[0xB] = true; break;
                    case SDL_SCANCODE_V: chip8.keypad[0xF] = true; break;
                    case SDL_SCANCODE_ESCAPE: chip8.state = emulator_state::quit; break;
                    case SDL_SCANCODE_F1:
                    {
                        chip8.state == emulator_state::running ? chip8.state = emulator_state::paused : chip8.state = emulator_state::running;
                        break;
                    }
                    case SDL_SCANCODE_F6:
                    {
                        chip8.load_previous_test_rom();
                        break;
                    }
                    case SDL_SCANCODE_F7:
                    {
                        chip8.load_next_test_rom();
                        break;
                    }
                }
                break;
            }
            case SDL_KEYUP:
            {
                switch (event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_1: chip8.keypad[0x1] = false; break;
                    case SDL_SCANCODE_2: chip8.keypad[0x2] = false; break;
                    case SDL_SCANCODE_3: chip8.keypad[0x3] = false; break;
                    case SDL_SCANCODE_4: chip8.keypad[0xC] = false; break;
                    case SDL_SCANCODE_Q: chip8.keypad[0x4] = false; break;
                    case SDL_SCANCODE_W: chip8.keypad[0x5] = false; break;
                    case SDL_SCANCODE_E: chip8.keypad[0x6] = false; break;
                    case SDL_SCANCODE_R: chip8.keypad[0xD] = false; break;
                    case SDL_SCANCODE_A: chip8.keypad[0x7] = false; break;
                    case SDL_SCANCODE_S: chip8.keypad[0x8] = false; break;
                    case SDL_SCANCODE_D: chip8.keypad[0x9] = false; break;
                    case SDL_SCANCODE_F: chip8.keypad[0xE] = false; break;
                    case SDL_SCANCODE_Z: chip8.keypad[0xA] = false; break;
                    case SDL_SCANCODE_X: chip8.keypad[0x0] = false; break;
                    case SDL_SCANCODE_C: chip8.keypad[0xB] = false; break;
                    case SDL_SCANCODE_V: chip8.keypad[0xF] = false; break;
                }
                break;
            }
        }
    }
}

void sdl2_handler::extract_rgba(uint32 color, uint8& r, uint8& g, uint8& b, uint8& a)
{
    r = (color >> 24) & 0xFF;
    g = (color >> 16) & 0xFF;
    b = (color >> 8)  & 0xFF;
    a = color         & 0xFF;
}
