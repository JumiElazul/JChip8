#include "sdl2_handler.h"
#include "emulator_config.h"
#include "JChip8.h"
#include "typedefs.h"
#include <SDL2/SDL.h>
#include <iostream>

sdl2_handler::sdl2_handler(const emulator_config& config)
    : _window(nullptr)
    , _renderer(nullptr)
    , _window_width(640)
    , _window_height(320)
    , _window_scale(2.0f)
    , _config(config)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << '\n';
        exit(1);
    }

    _window = SDL_CreateWindow("JChip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _window_width * _window_scale, _window_height * _window_scale, SDL_WINDOW_SHOWN);
    if (!_window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << '\n';
        exit(1);
    }

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer)
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << '\n';
        exit(1);
    }

    _want.freq = config.frequency;
    _want.format = AUDIO_S16LSB;
    _want.channels = 1;
    _want.samples = 4096;
    _want.callback = audio_callback;
    _want.userdata = (void*)&config;

    _audio_device = SDL_OpenAudioDevice(nullptr, 0, &_want, &_have, 0);
    if (!_audio_device)
    {
        std::cerr << "Audio device could not be created! SDL_Error: " << SDL_GetError() << '\n';
        exit(1);
    }

    if (_want.freq != _have.freq)
    {
        std::cerr << "Audio frequency requested in config is not available: " << SDL_GetError() << '\n';
        exit(1);
    }
}

sdl2_handler::~sdl2_handler()
{
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_CloseAudioDevice(_audio_device);
    SDL_Quit();
}

uint64 sdl2_handler::time() const noexcept
{
    return SDL_GetPerformanceCounter();
}

uint64 sdl2_handler::performance_freq() const noexcept
{
    return SDL_GetPerformanceFrequency();
}

void sdl2_handler::delay(uint32 time_ms) const noexcept
{
    SDL_Delay(time_ms);
}

SDL_Window* sdl2_handler::window() const noexcept { return _window; }
SDL_Renderer* sdl2_handler::renderer() const noexcept { return _renderer; }

void sdl2_handler::draw_graphics(JChip8& chip8)
{
    bool* gfx = &chip8.graphics[0];
    float scale_x = (_window_width * _window_scale)  / GRAPHICS_WIDTH;
    float scale_y = (_window_height * _window_scale) / GRAPHICS_HEIGHT;

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
                switch (event.key.keysym.sym)
                {
                    case SDLK_1: chip8.keypad[0x1] = true; break;
                    case SDLK_2: chip8.keypad[0x2] = true; break;
                    case SDLK_3: chip8.keypad[0x3] = true; break;
                    case SDLK_4: chip8.keypad[0xC] = true; break;
                    case SDLK_q: chip8.keypad[0x4] = true; break;
                    case SDLK_w: chip8.keypad[0x5] = true; break;
                    case SDLK_f: chip8.keypad[0x6] = true; break;
                    case SDLK_p: chip8.keypad[0xD] = true; break;
                    case SDLK_a: chip8.keypad[0x7] = true; break;
                    case SDLK_r: chip8.keypad[0x8] = true; break;
                    case SDLK_s: chip8.keypad[0x9] = true; break;
                    case SDLK_t: chip8.keypad[0xE] = true; break;
                    case SDLK_z: chip8.keypad[0xA] = true; break;
                    case SDLK_x: chip8.keypad[0x0] = true; break;
                    case SDLK_c: chip8.keypad[0xB] = true; break;
                    case SDLK_d: chip8.keypad[0xF] = true; break;
                    case SDLK_ESCAPE: chip8.state = emulator_state::quit; break;
                    case SDLK_F1:
                    {
                        chip8.state == emulator_state::running ? chip8.state = emulator_state::paused : chip8.state = emulator_state::running;
                        break;
                    }
                    case SDLK_F6:
                    {
                        chip8.load_previous_test_rom();
                        break;
                    }
                    case SDLK_F7:
                    {
                        chip8.load_next_test_rom();
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case SDL_KEYUP:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_1: chip8.keypad[0x1] = false; break;
                    case SDLK_2: chip8.keypad[0x2] = false; break;
                    case SDLK_3: chip8.keypad[0x3] = false; break;
                    case SDLK_4: chip8.keypad[0xC] = false; break;
                    case SDLK_q: chip8.keypad[0x4] = false; break;
                    case SDLK_w: chip8.keypad[0x5] = false; break;
                    case SDLK_f: chip8.keypad[0x6] = false; break;
                    case SDLK_p: chip8.keypad[0xD] = false; break;
                    case SDLK_a: chip8.keypad[0x7] = false; break;
                    case SDLK_r: chip8.keypad[0x8] = false; break;
                    case SDLK_s: chip8.keypad[0x9] = false; break;
                    case SDLK_t: chip8.keypad[0xE] = false; break;
                    case SDLK_z: chip8.keypad[0xA] = false; break;
                    case SDLK_x: chip8.keypad[0x0] = false; break;
                    case SDLK_c: chip8.keypad[0xB] = false; break;
                    case SDLK_d: chip8.keypad[0xF] = false; break;
                    default:
                        break;
                }
                break;
            }
        }
    }
}

void sdl2_handler::play_device(bool play) const
{
    play ? SDL_PauseAudioDevice(_audio_device, 0) : SDL_PauseAudioDevice(_audio_device, 1);
}

void sdl2_handler::extract_rgba(uint32 color, uint8& r, uint8& g, uint8& b, uint8& a) const
{
    r = (color >> 24) & 0xFF;
    g = (color >> 16) & 0xFF;
    b = (color >> 8)  & 0xFF;
    a = color         & 0xFF;
}

void sdl2_handler::audio_callback(void* userdata, uint8* stream, int len)
{
    emulator_config* config = static_cast<emulator_config*>(userdata);

    int16* audio_data = (int16*)stream;
    static uint32 running_sample_index = 0;
    const int32 square_wave_period = config->frequency / config->wave_frequency;
    const int32 half_square_wave_period = square_wave_period / 2;

    for (int i = 0; i < len / 2; ++i)
    {
        audio_data[i] = ((running_sample_index++ / half_square_wave_period) % 2) ?  config->volume : -config->volume;
    }
}

