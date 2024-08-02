#include "emulator_config.h"
#include "typedefs.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>

std::string config_filepath = std::filesystem::current_path().string().append("/config.json");

emulator_config load_configuration_file()
{
    if (!std::filesystem::exists(config_filepath))
    {
        return create_default_config_file();
    }
    else
    {
        nlohmann::json json;
        std::ifstream file(config_filepath);

        if (!file)
        {
            throw std::runtime_error("Configuration file exists but could not be loaded");
        }

        file >> json;
        emulator_config config = json.get<emulator_config>();
        return config;
    }
}

emulator_config create_default_config_file()
{
    emulator_config default_config;
    nlohmann::json json = default_config;

    std::fstream file(config_filepath, std::fstream::out);
    if (!file)
    {
        throw std::runtime_error("Failed to create the default configuration file");
    }

    file << json.dump(4);
    return default_config;
}

std::string to_hex(uint32 value)
{
}

uint32 from_hex(const std::string& hex_str)
{
}

void to_json(nlohmann::json& j, const emulator_config& config)
{
    j = nlohmann::json
    {
        {"bg_color", to_hex(config.bg_color)},
        {"fg_color", to_hex(config.fg_color)},
        {"pixel_outlines", config.pixel_outlines},
        {"frequency", config.frequency},
        {"wave_frequency", config.wave_frequency},
        {"volume", config.volume},
    };
}

void from_json(const nlohmann::json& j, emulator_config& config)
{
    config.bg_color = from_hex(j.at("bg_color").get<std::string>());
    config.fg_color = from_hex(j.at("fg_color").get<std::string>());
    j.at("pixel_outlines").get_to(config.pixel_outlines);
    j.at("frequency").get_to(config.frequency);
    j.at("wave_frequency").get_to(config.wave_frequency);
    j.at("volume").get_to(config.volume);
}
