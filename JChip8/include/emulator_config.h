#ifndef JUMI_CHIP8_EMULATOR_CONFIG_H
#define JUMI_CHIP8_EMULATOR_CONFIG_H
#include "typedefs.h"

struct emulator_config
{
    uint32 bg_color = 0x003366;
    uint32 fg_color = 0x66CCFF;
    bool pixel_outlines = true;
};

#endif

