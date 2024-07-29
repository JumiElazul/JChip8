#ifndef JUMI_JCHIP8_EMULATOR_H
#define JUMI_JCHIP8_EMULATOR_H
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <random>
#include <vector>
#include <unordered_map>
#include "typedefs.h"

//0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
//0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
//0x200-0xFFF - Program ROM and work RAM

//Memory: CHIP-8 has direct access to up to 4 kilobytes of RAM
//Display: 64 x 32 pixels (or 128 x 64 for SUPER-CHIP) monochrome, ie. black or white
//A program counter, often called just �PC�, which points at the current instruction in memory
//One 16-bit index register called �I� which is used to point at locations in memory
//A stack for 16-bit addresses, which is used to call subroutines/functions and return from them
//An 8-bit delay timer which is decremented at a rate of 60 Hz (60 times per second) until it reaches 0
//An 8-bit sound timer which functions like the delay timer, but which also gives off a beeping sound as long as it�s not 0
//16 8-bit (one byte) general-purpose variable registers numbered 0 through F hexadecimal, ie. 0 through 15 in decimal, called V0 through VF
//VF is also used as a flag register; many instructions will set it to either 1 or 0 based on some rule, for example using it as a carry flag

//#define DEBUG_INSTRUCTION

#define DRAW_INSTRUCTION 0x0D

static constexpr uint16 MEMORY_SIZE = 4096;
static constexpr uint16 GRAPHICS_WIDTH = 64;
static constexpr uint16 GRAPHICS_HEIGHT = 32;

struct instruction
{
    uint16 opcode;
    uint16 NNN;     // 12-bit value
    uint8 NN;       // 8-bit constant
    uint8 N;        // 4-bit constant
    uint8 X;        // 4-bit register identifier
    uint8 Y;        // 4-bit register identifier
};

enum class pixel_state
{
    off,
    on,
};

enum class emulator_state
{
    running,
    paused,
    quit,
};

struct ROM
{
    std::string filepath;
    std::string name;
    uint16 size;
};

class instruction_history
{
static constexpr uint32 MAX_INSTRUCTION_HISTORY = 1024;
public:
    instruction_history();
    void add_instruction(uint16 memory_address, instruction& instr);
    void log_last_instruction() const noexcept;
    [[nodiscard]] const std::pair<uint16, instruction>& get_instruction(uint32 index) const;
    [[nodiscard]] uint32 get_size() const noexcept;
    void clear();

private:
    std::array<std::pair<uint16, instruction>, MAX_INSTRUCTION_HISTORY> _instructions;
    uint32 _ip;
    std::unordered_map<uint16, std::string> _instruction_descriptions;

    const std::string& get_instruction_description(uint16 opcode) const noexcept;
};

class JChip8
{
public:
    uint8 memory[MEMORY_SIZE];
    uint8 V[16];
    uint16 pc;
    bool graphics[GRAPHICS_WIDTH * GRAPHICS_HEIGHT];
    uint16 stack[16];
    uint16 sp;
    uint8 delay_timer;
    uint8 sound_timer;
    uint16 I;
    bool keypad[16];
    emulator_state state;
    uint16 ips;

    JChip8(uint16 ips_ = 700);
    ~JChip8();

    [[nodiscard]] bool draw_flag() const noexcept;
    [[nodiscard]] instruction fetch_instruction();
    void emulate_cycle();
    void execute_instruction(instruction& instr);
    void update_timers();
    void load_game(const ROM& rom);
    void reset_draw_flag();
    void load_next_test_rom();
    void load_previous_test_rom();
    const instruction& current_instruction() const noexcept;

private:
    std::vector<ROM> _test_roms;
    int16 _current_rom;
    bool _draw_flag;
    instruction_history* _instruction_history;
    instruction _current_instruction;
    std::mt19937 _rng;
    void init_state();
    void load_fontset();
    void clear_graphics_buffer();
    uint8 generate_random_number();
    void load_test_suite_roms();
 };

#endif
