#ifndef JUMI_CHIP8_SDL_HANDLER_H
#define JUMI_CHIP8_SDL_HANDLER_H
#include <cstdint>
#include "typedefs.h"

struct SDL_Window;
struct SDL_Renderer;
struct ROM;
struct emulator_config;

class JChip8;

class sdl2_handler
{
public:
    sdl2_handler(const emulator_config& config);
    ~sdl2_handler();
    sdl2_handler(const sdl2_handler&) = delete;
    sdl2_handler& operator=(const sdl2_handler&) = delete;
    sdl2_handler(sdl2_handler&&) = delete;
    sdl2_handler& operator=(sdl2_handler&&) = delete;

    uint64 time() const noexcept;
    uint64 performance_freq() const noexcept;
    void delay(uint32 time_ms) const noexcept;
    SDL_Window* window() const noexcept;
    SDL_Renderer* renderer() const noexcept;
    void draw_graphics(JChip8& chip8);
    void handle_input(JChip8& chip8);

private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    uint32 _window_width;
    uint32 _window_height;
    float _window_scale;
    const emulator_config& _config;

    void extract_rgba(uint32 color, uint8& r, uint8& g, uint8& b, uint8& a) const;
};

#endif

