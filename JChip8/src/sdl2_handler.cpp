#include "sdl2_handler.h"
#include "JChip8.h"
#include <SDL2/SDL.h>
#include <iostream>

sdl2_handler::sdl2_handler(float scale)
    : _window(nullptr)
    , _renderer(nullptr)
    , _scale(scale)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    int window_width = 64 * _scale;
    int window_height = 32 * _scale;

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

std::uint32_t sdl2_handler::time() const noexcept
{
    return SDL_GetTicks();
}

SDL_Window* sdl2_handler::window() const noexcept { return _window; }
SDL_Renderer* sdl2_handler::renderer() const noexcept { return _renderer; }

void sdl2_handler::draw_graphics(unsigned char* graphics, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        if (graphics[i] == 0)
            SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
        else
            SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

        int x_origin = (i % 64) * _scale;
        int y_origin = (i / 64) * _scale;
        int width = _scale;
        int height = _scale;

        SDL_Rect rect = { x_origin, y_origin, width, height };
        SDL_RenderFillRect(_renderer, &rect);
    }
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
                chip8.set_state(emulator_state::quit);
                break;
            }
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_1: chip8.set_key(0x1, true); break;
                    case SDLK_2: chip8.set_key(0x2, true); break;
                    case SDLK_3: chip8.set_key(0x3, true); break;
                    case SDLK_4: chip8.set_key(0xC, true); break;
                    case SDLK_q: chip8.set_key(0x4, true); break;
                    case SDLK_w: chip8.set_key(0x5, true); break;
                    case SDLK_e: chip8.set_key(0x6, true); break;
                    case SDLK_r: chip8.set_key(0xD, true); break;
                    case SDLK_a: chip8.set_key(0x7, true); break;
                    case SDLK_s: chip8.set_key(0x8, true); break;
                    case SDLK_d: chip8.set_key(0x9, true); break;
                    case SDLK_f: chip8.set_key(0xE, true); break;
                    case SDLK_z: chip8.set_key(0xA, true); break;
                    case SDLK_x: chip8.set_key(0x0, true); break;
                    case SDLK_c: chip8.set_key(0xB, true); break;
                    case SDLK_v: chip8.set_key(0xF, true); break;
                    case SDLK_p: chip8.set_state(emulator_state::paused); break;
                }
                break;
            }
            case SDL_KEYUP:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_1: chip8.set_key(0x1, false); break;
                    case SDLK_2: chip8.set_key(0x2, false); break;
                    case SDLK_3: chip8.set_key(0x3, false); break;
                    case SDLK_4: chip8.set_key(0xC, false); break;
                    case SDLK_q: chip8.set_key(0x4, false); break;
                    case SDLK_w: chip8.set_key(0x5, false); break;
                    case SDLK_e: chip8.set_key(0x6, false); break;
                    case SDLK_r: chip8.set_key(0xD, false); break;
                    case SDLK_a: chip8.set_key(0x7, false); break;
                    case SDLK_s: chip8.set_key(0x8, false); break;
                    case SDLK_d: chip8.set_key(0x9, false); break;
                    case SDLK_f: chip8.set_key(0xE, false); break;
                    case SDLK_z: chip8.set_key(0xA, false); break;
                    case SDLK_x: chip8.set_key(0x0, false); break;
                    case SDLK_c: chip8.set_key(0xB, false); break;
                    case SDLK_v: chip8.set_key(0xF, false); break;
                }
                break;
            }
        }
    }
}
