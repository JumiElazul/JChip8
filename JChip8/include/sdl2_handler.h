#ifndef JUMI_CHIP8_SDL_HANDLER_H
#define JUMI_CHIP8_SDL_HANDLER_H
#include <cstdint>
#include "typedefs.h"

struct SDL_Window;
struct SDL_Renderer;

class JChip8;

class sdl2_handler
{
public:
    sdl2_handler();
    ~sdl2_handler();
    sdl2_handler(const sdl2_handler&) = delete;
    sdl2_handler& operator=(const sdl2_handler&) = delete;
    sdl2_handler(sdl2_handler&&) = delete;
    sdl2_handler& operator=(sdl2_handler&&) = delete;

    std::uint32_t time() const noexcept;
    SDL_Window* window() const noexcept;
    SDL_Renderer* renderer() const noexcept;
    void draw_graphics(bool* graphics, size_t length);
    void handle_input(JChip8& chip8);

private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
};

#endif

