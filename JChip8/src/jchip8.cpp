#include "jchip8.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <string>
#include <vector>

instruction_history::instruction_history() 
    : _instructions{ }
    , _ip{ MAX_INSTRUCTION_HISTORY - 1 }
    , _instruction_descriptions{
        {0x00E0, "Clear the display"},
        {0x00EE, "Return from a subroutine"},
        {0x1000, "Jump to address NNN"},
        {0x2000, "Call subroutine at NNN"},
        {0x3000, "Skip next instruction if Vx equals NN"},
        {0x4000, "Skip next instruction if Vx not equal to NN"},
        {0x5000, "Skip next instruction if Vx equals Vy"},
        {0x6000, "Set Vx to NN"},
        {0x7000, "Add NN to Vx"},
        {0x8000, "Set Vx to the value of Vy"},
        {0x8001, "Set Vx to Vx OR Vy"},
        {0x8002, "Set Vx to Vx AND Vy"},
        {0x8003, "Set Vx to Vx XOR Vy"},
        {0x8004, "Add Vy to Vx, set VF to 1 if there's a carry, 0 if not"},
        {0x8005, "Subtract Vy from Vx, set VF to 0 if there's a borrow, 1 if not"},
        {0x8006, "Store the least significant bit of Vx in VF, then shift Vx to the right by 1"},
        {0x8007, "Set Vx to Vy minus Vx, set VF to 0 if there's a borrow, 1 if not"},
        {0x800E, "Store the most significant bit of Vx in VF, then shift Vx to the left by 1"},
        {0x9000, "Skip next instruction if Vx not equal to Vy"},
        {0xA000, "Set I to the address NNN"},
        {0xB000, "Jump to the address NNN plus V0"},
        {0xC000, "Set Vx to the result of a bitwise AND operation on a random number and NN"},
        {0xD000, "Draw a sprite at coordinate (Vx, Vy) with a width of 8 pixels and a height of N pixels"},
        {0xE09E, "Skip the next instruction if the key stored in Vx is pressed"},
        {0xE0A1, "Skip the next instruction if the key stored in Vx is not pressed"},
        {0xF007, "Set Vx to the value of the delay timer"},
        {0xF00A, "Wait for a key press, store the value of the key in Vx"},
        {0xF015, "Set the delay timer to Vx"},
        {0xF018, "Set the sound timer to Vx"},
        {0xF01E, "Add Vx to I"},
        {0xF029, "Set I to the location of the sprite for the character in Vx"},
        {0xF033, "Store the binary-coded decimal representation of Vx at the addresses I, I+1, and I+2"},
        {0xF055, "Store registers V0 through Vx in memory starting at location I"},
        {0xF065, "Read registers V0 through Vx from memory starting at location I"}
    }
{

}

void instruction_history::add_instruction(uint16 memory_address, instruction& instr)
{
    _ip = (_ip + 1) % MAX_INSTRUCTION_HISTORY;
    _instructions[_ip] = std::make_pair(memory_address, instr);
}

void instruction_history::log_last_instruction() const noexcept
{
    const std::pair<uint16, instruction>& last_instruction = _instructions[_ip];
    std::ios_base::fmtflags flags = std::cout.flags();
    std::cout << "Address: [0x" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << last_instruction.first
              << "]   Instruction: [0x" << std::setw(4) << std::setfill('0') << last_instruction.second.opcode
              << "]   Description: [" << get_instruction_description(last_instruction.second.opcode) << "]\n";
    std::cout.flags(flags);
}

const std::pair<uint16, instruction>& instruction_history::get_instruction(uint32 index) const
{
    if (index >= MAX_INSTRUCTION_HISTORY)
        throw std::out_of_range("Index out of range");

    return _instructions[index];
}

uint32 instruction_history::get_size() const noexcept { return MAX_INSTRUCTION_HISTORY; }

void instruction_history::clear()
{
    std::fill(_instructions.begin(), _instructions.end(), std::make_pair<uint16, instruction>(0x00, instruction()));
}

const std::string& instruction_history::get_instruction_description(uint16 opcode) const noexcept
{
    uint16_t masked_opcode = opcode & 0xF000;
    if (_instruction_descriptions.find(masked_opcode) != _instruction_descriptions.end())
        return _instruction_descriptions.at(masked_opcode);

    masked_opcode = opcode & 0xF00F;
    if (_instruction_descriptions.find(masked_opcode) != _instruction_descriptions.end())
        return _instruction_descriptions.at(masked_opcode);

    masked_opcode = opcode & 0xF0FF;
    if (_instruction_descriptions.find(masked_opcode) != _instruction_descriptions.end()) 
        return _instruction_descriptions.at(masked_opcode);

    return "Unknown opcode";
}

JChip8::JChip8(uint16 ips_)
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
    , state{ emulator_state::running }
    , ips{ ips_ }
    , _test_roms{}
    , _current_rom{ -1 }
    , _draw_flag{ false }
    , _instruction_history{ new instruction_history() }
    , _rng(std::random_device()())
{
    load_test_suite_roms();
    load_fontset();
}

JChip8::~JChip8()
{
    delete _instruction_history;
}

bool JChip8::draw_flag() const noexcept { return _draw_flag; }

instruction JChip8::fetch_instruction()
{
    instruction instr
    {
        .opcode = static_cast<uint16>(memory[pc] << 8 | memory[pc + 1]),
        .NNN    = static_cast<uint16>(instr.opcode & 0x0FFF),
        .NN     = static_cast<uint8>(instr.opcode & 0x00FF),
        .N      = static_cast<uint8>(instr.opcode & 0x000F),
        .X      = static_cast<uint8>((instr.opcode & 0x0F00) >> 8),
        .Y      = static_cast<uint8>((instr.opcode & 0x00F0) >> 4)
    };

    return instr;
}

void JChip8::emulate_cycle()
{
    instruction instr = fetch_instruction();
    _instruction_history->add_instruction(pc, instr);
    pc += 2;

#ifdef DEBUG_INSTRUCTIONS
    _instruction_history->log_last_instruction();
#endif

    execute_instruction(instr);

    // Update timers
    update_timers();
}

void JChip8::execute_instruction(instruction& instr)
{
    switch (instr.opcode >> 12)
    {
        case 0x00:
            if (instr.NN == 0xE0)
            {
                clear_graphics_buffer();
            }
            else if (instr.NN == 0xEE)
            {
                pc = stack[--sp];
            }
            break;

        case 0x01:
            pc = instr.NNN;
            break;

        case 0x02:
            stack[sp++] = pc;
            pc = instr.NNN;
            break;

        case 0x03:
            if (V[instr.X] == instr.NN) pc += 2;
            break;

        case 0x04:
            if (V[instr.X] != instr.NN) pc += 2;
            break;

        case 0x05:
            if (V[instr.X] == V[instr.Y])
                pc += 2;
            break;

        case 0x06:
            V[instr.X] = instr.NN;
            break;

        case 0x07:
            V[instr.X] += instr.NN;
            break;

        case 0x08:
            switch (instr.N)            
            {
                case 0x00:
                    V[instr.X] = V[instr.Y];
                    break;

                case 0x01:
                    V[instr.X] |= V[instr.Y];
                    break;

                case 0x02:
                    V[instr.X] &= V[instr.Y];
                    break;

                case 0x03:
                    V[instr.X] ^= V[instr.Y];
                    break;

                case 0x04:
                {
                    uint16 sum = V[instr.X] + V[instr.Y];
                    if (sum > 0xFF) V[0xF] = 1;
                    else            V[0xF] = 0;

                    V[instr.X] = sum & 0xFF;
                    break;
                }

                case 0x05:
                {
                    V[instr.X] > V[instr.Y] ? V[0xF] = 1 : V[0xF] = 0;
                    V[instr.X] -= V[instr.Y];
                    break;
                }

                case 0x06:
                    V[0xF] = V[instr.Y] & 0x01;
                    V[instr.X] >>= 1;
                    break;

                case 0x07:
                    V[instr.Y] > V[instr.X] ? V[0xF] = 1 : V[0xF] = 0;
                    V[instr.X] = V[instr.Y] - V[instr.X];
                    break;

                case 0x0E:
                    V[0xF] = (V[instr.X] & 0x80) >> 7;
                    V[instr.X] = V[instr.X] << 1;
                    break;
            }
            break;

        case 0x09:
            if (V[instr.X] != V[instr.Y]) pc += 2;
            break;

        case 0x0A:
            I = instr.NNN;
            break;

        case 0x0B:
            pc = instr.NNN + V[0];
            break;

        case 0x0C:
        {
            uint8 rnd_num = generate_random_number();
            V[instr.X] = rnd_num & instr.NN;
            break;
        }

        // --------------------------------------------------
        //                  Draw Instruction
        // --------------------------------------------------
        case 0x0D:
        {
            // DXYN
            // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
            // Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not
            // change after the execution of this instruction. As described above, VF is set to 1 if any screen
            // pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen.
            V[0xF] = 0;
            _draw_flag = true;
            uint8 height = instr.N;

            for (uint8 i = 0; i < height; ++i)
            {
                uint8 sprite = memory[I + i];
                uint8 row = (V[instr.Y] + i);
                if (row >= GRAPHICS_HEIGHT)
                    break;

                for (int8 j = 0; j < 8; ++j)
                {
                    uint8 bit = (sprite & 0x80) >> 7;
                    uint8 col = (V[instr.X] + j) % GRAPHICS_WIDTH;
                    if (col >= GRAPHICS_WIDTH)
                        break;

                    uint16 offset = row * GRAPHICS_WIDTH + col;

                    if (bit == 1)
                    {
                        if (offset < sizeof(graphics))
                        {
                            if (graphics[offset] == static_cast<uint8>(pixel_state::on))
                            {
                                graphics[offset] = 0;
                                V[0xF] = 1;
                            }
                            else
                            {
                                graphics[offset] = 1;
                            }
                        }
                    }

                    sprite <<= 1;
                }
            }

            break;
        }

        case 0x0E:
            if (instr.NN == 0x9E)
            {
                uint8 key = V[instr.X];
                if (keypad[key])
                    pc += 2;
            }
            else if (instr.NN == 0xA1)
            {
                uint8 key = V[instr.X];
                if (!keypad[key])
                    pc += 2;
            }
            break;

        case 0x0F:
            switch (instr.NN)
            {
                case 0x07:
                    V[instr.X] = delay_timer;
                    break;

                case 0x0A:
                {
                    bool key_pressed = false;
                    for (uint8 i = 0; i < sizeof(keypad); ++i)
                    {
                        if (keypad[i])
                        {
                            V[instr.X] = i;
                            key_pressed = true;
                            break;
                        }
                    }
                    if (!key_pressed) pc -= 2;
                    break;
                }

                case 0x15:
                    delay_timer = V[instr.X];
                    break;

                case 0x18:
                    sound_timer = V[instr.X];
                    break;

                case 0x1E:
                    I += V[instr.X];
                    break;

                case 0x29:
                    break;

                case 0x33:
                {
                    uint8 decimal_value = V[instr.X];
                    memory[I] = decimal_value / 100;
                    memory[I + 1] = (decimal_value / 10) % 10;
                    memory[I + 2] = decimal_value % 10;
                    break;
                }

                case 0x55:
                {
                    uint8 len = instr.X;
                    for (uint8 i = 0; i <= len; ++i)
                    {
                        memory[I + i] = V[i];
                    }
                    break;
                }

                case 0x65:
                {
                    uint8 len = instr.X;
                    for (uint8 i = 0; i <= len; ++i)
                    {
                        V[i] = memory[I + i];
                    }
                    break;
                }
            }
            break;

        // --------------------------------------------------
        //                  Draw Instruction
        // --------------------------------------------------

        default:
            std::cout << "Unimplemented instruction";
            break;
    }
}

void JChip8::load_game(const ROM& rom)
{
    init_state();

    std::ifstream file(rom.filepath, std::ios::binary);
    if (!file) throw std::runtime_error("Could not open file");

    if (rom.size > (MEMORY_SIZE - 0x200)) throw std::runtime_error("File too large to fit in memory");

    for (size_t i = 0; i < rom.size; ++i)
    {
        memory[0x200 + i] = file.get();
    }
}

void JChip8::reset_draw_flag() { _draw_flag = false; }

void JChip8::load_next_test_rom()
{
    _current_rom += 1;
    _current_rom %= _test_roms.size();
    load_game(_test_roms[_current_rom]);
}

void JChip8::load_previous_test_rom()
{
    _current_rom -= 1;
    if (_current_rom < 0)
        _current_rom = _test_roms.size() - 1;
    load_game(_test_roms[_current_rom]);
}

void JChip8::init_state()
{
    memset(memory, 0, sizeof(memory));
    memset(V, 0, sizeof(V));
    pc = 0x200;
    memset(graphics, 0, sizeof(graphics));
    memset(stack, 0, sizeof(stack));
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;
    I = 0;
    memset(keypad, 0, sizeof(keypad));
    state = emulator_state::running;

    _instruction_history->clear();
}

void JChip8::load_fontset()
{
    static uint8 fontset[80] =
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

void JChip8::update_timers()
{
    static std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();

    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    uint64 elapsed_time = static_cast<uint64>(std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count());

    // The timer loop happens at 60hz
    if (elapsed_time >= 1000 / 60)
    {
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

        last_time = current_time;
    }
}

void JChip8::clear_graphics_buffer()
{
    memset(graphics, false, sizeof(graphics));
    _draw_flag = true;
}

uint8 JChip8::generate_random_number()
{
    static std::uniform_int_distribution<int> uid(0, std::numeric_limits<uint8>::max());
    return uid(_rng);
}

void JChip8::load_test_suite_roms()
{
    std::filesystem::path path = std::filesystem::current_path().append("test_suite_roms");

    auto get_rom_size = [&](const std::string& filepath) {
        std::ifstream rom_file(filepath, std::ios::binary);
        if (!rom_file) throw std::runtime_error("Could not open file");

        rom_file.seekg(0, std::ios::end);
        uint16 size = static_cast<uint16>(rom_file.tellg());
        return size;
    };

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".ch8")
            {
                ROM rom =
                {
                    .filepath = entry.path().string(),
                    .name = entry.path().filename().replace_extension("").string(),
                    .size = get_rom_size(rom.filepath)
                };

                _test_roms.push_back(rom);
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception swallower: " << e.what() << '\n';
    }
}

