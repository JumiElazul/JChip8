#include "jchip8.h"
#include <fstream>
#include <iostream>

JChip8::JChip8()
    : _memory{ 0 }
    , _V{ 0 }
    , _pc{ 0x200 }
    , _graphics{ 0 }
    , _stack{ 0 }
    , _sp{ 0 }
    , _delay_timer{ 0 }
    , _sound_timer{ 0 }
    , _I{ 0 }
    , _keypad{ 0 }
    , _ips{ 700 }
    , _draw_flag{ false }
{
    load_fontset();
}

void JChip8::emulate_cycle()
{
    opcode op = fetch_opcode();

    // Decode and execute opcode
    decode_opcode(op);

    // Update timers
    if (_delay_timer > 0)
        --_delay_timer;

    if (_sound_timer > 0)
    {
        if (_sound_timer == 1)
        {
            std::cout << "BEEP!\n";
        }
        --_sound_timer;
    }
}

opcode JChip8::fetch_opcode()
{
    opcode code = _memory[_pc] << 8 | _memory[_pc + 1];
    _pc += 2;
    return code;
}

void JChip8::decode_opcode(opcode op)
{
    switch (op & 0xF000)
    {
        case 0xA000:
        {

            _I = op & 0x0FFF;
            _pc += 2;
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
    file.seekg(0, std::ios::beg);

    for (size_t i = 0; i < size; ++i)
    {
        _memory[0x200 + i] = file.get();
    }
}

bool JChip8::draw_flag() const noexcept { return _draw_flag; }

std::pair<unsigned char*, unsigned short> JChip8::graphics() noexcept
{
    return std::make_pair<unsigned char*, unsigned short>(_graphics, GRAPHICS_WIDTH * GRAPHICS_HEIGHT);
}

emulator_state JChip8::state() const noexcept { return _state; }
void JChip8::set_state(emulator_state state) { _state = state; }

void JChip8::set_key(unsigned char key, bool pressed)
{
    pressed ? _keypad[key] = 1 : _keypad[key] = 0;
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
        _memory[0x050 + i] = fontset[i];
    }
}

void JChip8::clear_screen()
{
    for (int i = 0; i < GRAPHICS_WIDTH * GRAPHICS_HEIGHT; ++i)
    {
        _graphics[i] = 0;
    }
}
