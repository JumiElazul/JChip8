#include "imgui_handler.h"
#include "sdl2_handler.h"
#include "emulator_config.h"
#include "JChip8.h"
#include "typedefs.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

imgui_handler::imgui_handler(const sdl2_handler& sdl_handler)
    : _menu_height{  }
    , _reload_config{ false }
    , _init_default_config{ false }
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(sdl_handler.window(), sdl_handler.renderer());
    ImGui_ImplSDLRenderer2_Init(sdl_handler.renderer());
}

imgui_handler::~imgui_handler()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

const uint16& imgui_handler::get_window_height() const noexcept
{
    return _menu_height;
}

bool imgui_handler::reload_config() const noexcept
{
    return _reload_config;
}

bool imgui_handler::init_default_config() const noexcept
{
    return _init_default_config;
}

void imgui_handler::begin_frame(const sdl2_handler& sdl_handler)
{
    _reload_config = false;
    _init_default_config = false;

    ImGui_ImplSDL2_NewFrame(sdl_handler.window());
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui::NewFrame();
}

void imgui_handler::draw_gui(JChip8& chip8)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Game"))
        {
            if (ImGui::MenuItem("Load ROM"))
            {
                std::string rom_path = open_file_dialog();
                chip8.load_ROM(rom_path.c_str());
            } ImGui::Separator();

            if (ImGui::MenuItem("Unload ROM"))
            {
                chip8.unload_ROM();
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Configuration"))
        {
            static bool enable_superchip = false;
            if (ImGui::Checkbox("Enable Superchip", &enable_superchip))
            {
                if (enable_superchip)
                    chip8.mode = chip8_mode::superchip;
                else
                    chip8.mode = chip8_mode::chip8;

            } ImGui::Separator();
            if (ImGui::MenuItem("Open Config File"))
            {
                open_config_file(config::s_config_filepath.c_str());
            } ImGui::Separator();
            if (ImGui::MenuItem("Reload Config File"))
            {
                _reload_config = true;
            } ImGui::Separator();
            if (ImGui::MenuItem("Reset Config to Defaults"))
            {
                _init_default_config = true;
                _reload_config = true;
            } ImGui::Separator();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Exit"))
        {
            chip8.state = emulator_state::quit;
            ImGui::EndMenu();
        }

        _menu_height = static_cast<uint16>(ImGui::GetFrameHeight());
        ImGui::EndMainMenuBar();
    }
}

void imgui_handler::end_frame()
{
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void imgui_handler::process_event(SDL_Event* event) const
{
    ImGui_ImplSDL2_ProcessEvent(event);
}

std::string imgui_handler::open_file_dialog() const
{
    const char* filter_patterns[] = { "*.ch8" };
    const char* selected_file = tinyfd_openFileDialog( "Select a ROM file", "", 1, filter_patterns, nullptr, 0);

    if (selected_file)
    {
        return std::string(selected_file);
    }
    else
    {
        // File dialog was most likely cancelled
    }
}

void imgui_handler::open_config_file(const char* filepath)
{
#if defined(_WIN32)
    ShellExecuteA(nullptr, "open", filepath, nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__linux__)
    std::string command = "xdg-open " + std::string(filepath);
    std::system(command.c_str());
#elif defined(__APPLE__) // :: Untested ::
    std::string command = "open " + std::string(filepath);
    std::system(command.c_str());
#endif
}
