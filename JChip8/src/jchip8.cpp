#include "jchip8.h"
#include <fstream>
#include <iostream>

JChip8::JChip8()
    : memory{ 0 }
    , V{ 0 }
    , pc{ 0x200 }
    , graphics{ 0 }
    , stack{ 0 }
    , sp{ 0 }
    , delay_timer{ 0 }
    , sound_timer{ 0 }
    , I{ 0 }
    , keypad{ 0 }
{
    load_fontset();
}

void JChip8::emulate_cycle()
{
    opcode op = fetch_opcode();

    // Decode and execute opcode
    decode_opcode(op);

    // Update timers
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
        {
            std::cout << "BEEP!\n";
        }
        --sound_timer;
    }
}

opcode JChip8::fetch_opcode()
{
    opcode code = memory[pc] << 8 | memory[pc + 1];
    pc += 2;

    // TEMP
    if (pc > MEMORY_SIZE)
        pc = 0x200;

    return code;
}

void JChip8::decode_opcode(opcode op)
{
    switch (op & 0xF000)
    {
        case 0xA000:
        {

            I = op & 0x0FFF;
            pc += 2;
            break;
        }
    }
}

void JChip8::load_game(const char* game)
{
    std::ifstream file(game, std::ios::binary);

    if (!file) throw std::runtime_error("Could not open file");

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();

    if (size > (MEMORY_SIZE - 0x200)) throw std::runtime_error("File too large to fit in memory");

    file.seekg(0, std::ios::beg);

    for (size_t i = 0; i < size; ++i)
    {
        memory[0x200 + i] = file.get();
    }
}

void JChip8::clear_screen()
{
    for (int i = 0; i < GRAPHICS_WIDTH * GRAPHICS_HEIGHT; ++i)
    {
        graphics[i] = 0;
    }
}

void JChip8::load_fontset()
{
    unsigned char fontset[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < 80; ++i)
    {
        memory[0x050 + i] = fontset[i];
    }
}

