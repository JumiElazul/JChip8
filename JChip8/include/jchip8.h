#include <utility>
#ifndef JUMI_JCHIP8_H
#define JUMI_JCHIP8_H

//0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
//0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
//0x200-0xFFF - Program ROM and work RAM

//Memory: CHIP-8 has direct access to up to 4 kilobytes of RAM
//Display: 64 x 32 pixels (or 128 x 64 for SUPER-CHIP) monochrome, ie. black or white
//A program counter, often called just “PC”, which points at the current instruction in memory
//One 16-bit index register called “I” which is used to point at locations in memory
//A stack for 16-bit addresses, which is used to call subroutines/functions and return from them
//An 8-bit delay timer which is decremented at a rate of 60 Hz (60 times per second) until it reaches 0
//An 8-bit sound timer which functions like the delay timer, but which also gives off a beeping sound as long as it’s not 0
//16 8-bit (one byte) general-purpose variable registers numbered 0 through F hexadecimal, ie. 0 through 15 in decimal, called V0 through VF
//VF is also used as a flag register; many instructions will set it to either 1 or 0 based on some rule, for example using it as a carry flag

using opcode = unsigned short;

static constexpr unsigned short MEMORY_SIZE = 4096;
static constexpr unsigned short GRAPHICS_WIDTH = 64;
static constexpr unsigned short GRAPHICS_HEIGHT = 32;

enum class emulator_state
{
    running,
    paused,
    quit,
};

class JChip8
{
public:
    JChip8();

    void emulate_cycle();
    opcode fetch_opcode();
    void decode_opcode(opcode op);
    void load_game(const char* game);
    bool draw_flag() const noexcept;
    std::pair<unsigned char*, unsigned short> graphics() noexcept;
    emulator_state state() const noexcept;
    void set_state(emulator_state state);
    void set_key(unsigned char key, bool pressed);

private:
    unsigned char _memory[MEMORY_SIZE];
    unsigned char _V[16];
    unsigned short _pc;
    unsigned char _graphics[GRAPHICS_WIDTH * GRAPHICS_HEIGHT];
    unsigned short _stack[16];
    unsigned short _sp;
    unsigned char _delay_timer;
    unsigned char _sound_timer;
    unsigned short _I;
    unsigned char _keypad[16];
    unsigned short _ips;
    bool _draw_flag;
    emulator_state _state;

    void load_fontset();
    void clear_screen();
 };

#endif
