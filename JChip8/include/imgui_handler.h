#ifndef JUMI_CHIP8_IMGUI_HANDLER_H
#define JUMI_CHIP8_IMGUI_HANDLER_H
#include "typedefs.h"
#include <SDL2/SDL.h>
#include <string>

class sdl2_handler;
class JChip8;

class imgui_handler
{
public:
    imgui_handler(const sdl2_handler& sdl_handler);
    ~imgui_handler();
    imgui_handler(const imgui_handler&) = delete;
    imgui_handler& operator=(const imgui_handler&) = delete;
    imgui_handler(imgui_handler&&) = delete;
    imgui_handler& operator=(imgui_handler&&) = delete;

    [[nodiscard]] const uint16& get_window_height() const noexcept;
    [[nodiscard]] bool reload_config() const noexcept;
    [[nodiscard]] bool init_default_config() const noexcept;
    void begin_frame(const sdl2_handler& sdl_handler);
    void draw_gui(JChip8& chip8);
    void end_frame();
    void process_event(SDL_Event* event) const;

private:
    uint16 _menu_height;
    bool _reload_config;
    bool _init_default_config;

    std::string open_file_dialog() const;
    void open_config_file(const char* filepath);
};

#endif

