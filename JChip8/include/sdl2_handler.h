#ifndef JUMI_SDL_HANDLER_H
#define JUMI_SDL_HANDLER_H
#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;

class sdl2_handler
{
public:
    sdl2_handler();
    ~sdl2_handler();
    sdl2_handler(const sdl2_handler&) = delete;
    sdl2_handler& operator=(const sdl2_handler&) = delete;
    sdl2_handler(sdl2_handler&&) = delete;
    sdl2_handler& operator=(sdl2_handler&&) = delete;

    void setup_graphics();
    void setup_input();
    void set_keys();
    std::uint32_t time() const noexcept;
    SDL_Window* window() const noexcept;
    SDL_Renderer* renderer() const noexcept;

private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
};

#endif

