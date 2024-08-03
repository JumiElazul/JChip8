#ifndef JUMI_JCHIP8_EMULATOR_H
#define JUMI_JCHIP8_EMULATOR_H
#include "typedefs.h"
#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

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

#define DEBUG_INSTRUCTIONS

#define DRAW_INSTRUCTION 0x0D

static constexpr uint16 MEMORY_SIZE        = 4096;
static constexpr uint16 ROM_START_LOCATION = 0x200;
static constexpr uint16 GRAPHICS_WIDTH     = 64;
static constexpr uint16 GRAPHICS_HEIGHT    = 32;

class sdl2_handler;

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

struct instruction_debug
{
    std::string mode;
    uint16 memory_address;
    instruction instruction_;
};

class instruction_history
{
static constexpr uint32 MAX_INSTRUCTION_HISTORY = 1024;
public:
    instruction_history();
    void add_instruction(const std::string& mode, uint16 memory_address, instruction& instr);
    void log_last_instruction() const noexcept;
    [[nodiscard]] const instruction_debug& get_instruction(uint32 index) const;
    [[nodiscard]] uint32 get_size() const noexcept;
    void clear();

private:
    std::array<instruction_debug, MAX_INSTRUCTION_HISTORY> _instructions;
    uint32 _ip;
    std::unordered_map<uint16, std::string> _instruction_descriptions;

    const std::string& get_instruction_description(uint16 opcode) const noexcept;
};

enum class chip8_mode
{
    chip8,
    superchip
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
    chip8_mode mode;

    JChip8(uint16 ips_ = 700);
    ~JChip8();

    [[nodiscard]] bool draw_flag() const noexcept;
    [[nodiscard]] instruction fetch_instruction();
    [[nodiscard]] bool rom_loaded() const noexcept;
    void emulate_cycle();
    void execute_instruction(instruction& instr);
    void update_timers(const sdl2_handler& sdl_handler);
    void unload_ROM();
    void load_ROM(const char* rom_path);
    void reset_draw_flag();
    const instruction& current_instruction() const noexcept;

private:
    bool _rom_loaded;
    bool _draw_flag;
    instruction_history* _instruction_history;
    instruction _current_instruction;
    std::mt19937 _rng;

    void init_state();
    void load_fontset();
    void clear_graphics_buffer();
    uint8 generate_random_number();

    std::string mode_tostr();
 };

#endif
