#ifndef JUMI_CHIP8_EMULATOR_CONFIG_H
#define JUMI_CHIP8_EMULATOR_CONFIG_H
#include "typedefs.h"
#include <nlohmann/json.hpp>
#include <string>

struct emulator_config
{
    uint32 bg_color = 0x003366;
    uint32 fg_color = 0x66CCFF;
    bool pixel_outlines = true;
    uint32 frequency = 44100;
    uint32 wave_frequency = 440;
    int16 volume = 1200;
    uint16 instructions_per_second = 1000;
};

namespace config
{
    extern std::string s_config_filepath;
}

emulator_config load_configuration_file();
emulator_config create_default_config_file();
std::string to_hex(uint32 value);
uint32 from_hex(const std::string& hex_str);
void to_json(nlohmann::json& j, const emulator_config& p);
void from_json(const nlohmann::json& j, emulator_config& p);

#endif

