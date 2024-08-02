#include "emulator_config.h"
#include "typedefs.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>

using namespace config;
std::string config::s_config_filepath = std::filesystem::current_path().string().append("/config.json");

emulator_config load_configuration_file()
{
    if (!std::filesystem::exists(s_config_filepath))
    {
        return create_default_config_file();
    }
    else
    {
        nlohmann::json json;
        std::ifstream file(s_config_filepath);

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

    std::fstream file(s_config_filepath, std::fstream::out);
    if (!file)
    {
        throw std::runtime_error("Failed to create the default configuration file");
    }

    file << json.dump(4);
    return default_config;
}

std::string to_hex(uint32 value)
{
    std::stringstream ss;
    ss << "0x" << std::setw(6) << std::setfill('0') << std::uppercase << std::hex << (value & 0xFFFFFF);
    return ss.str();
}

uint32 from_hex(const std::string& hex_str)
{
    uint32_t value = 0;
    std::stringstream ss(hex_str);
    ss >> std::hex >> value;
    value = (value << 8) | 0x000000FF;
    return value;
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
        {"instructions_per_second", config.instructions_per_second},
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
    j.at("instructions_per_second").get_to(config.instructions_per_second);
}
