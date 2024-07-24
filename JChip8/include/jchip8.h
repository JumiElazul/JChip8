#ifndef JUMI_JCHIP8_H
#define JUMI_JCHIP8_H
#include <array>

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

static constexpr unsigned short MEMORY_SIZE = 4096;
static constexpr unsigned short GRAPHICS_WIDTH = 64;
static constexpr unsigned short GRAPHICS_HEIGHT = 32;
static constexpr unsigned int INSTRUCTION_MEMORY_SIZE = 1024;

struct instruction
{
    unsigned short opcode;
    unsigned short NNN;     // 12-bit value
    unsigned char NN;       // 8-bit constant
    unsigned char N;        // 4-bit constant
    unsigned char X;        // 4-bit register identifier
    unsigned char Y;        // 4-bit register identifier
};

enum class emulator_state
{
    running,
    paused,
    quit,
};

class JChip8
{
public:
    unsigned char memory[MEMORY_SIZE];
    unsigned char V[16];
    unsigned short pc;
    bool graphics[GRAPHICS_WIDTH * GRAPHICS_HEIGHT];
    unsigned short stack[16];
    unsigned short sp;
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short I;
    unsigned char keypad[16];
    emulator_state state;

    JChip8();

    void emulate_cycle();
    instruction fetch_instruction();
    void execute_instruction(instruction& instr);
    void load_game(const char* game);

private:
    std::array<std::pair<unsigned short, instruction>, INSTRUCTION_MEMORY_SIZE> _instruction_history;
    unsigned int _instruction_pointer;
    void load_fontset();
    void clear_screen();
    void update_instruction_history(instruction& instr);
 };

#endif
