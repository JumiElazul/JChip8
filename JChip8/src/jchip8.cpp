#include "jchip8.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <utility>

instruction_history::instruction_history() 
    : _instructions{ }
    , _ip{ MAX_INSTRUCTION_HISTORY - 1 }
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
              << "]   Description: [";

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
    , _draw_flag{ false }
    , _instruction_history{ new instruction_history() }
{
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

    _instruction_history->log_last_instruction();

    execute_instruction(instr);

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

//0NNN Calls machine code routine (RCA 1802 for COSMAC VIP) at address NNN. Not necessary for most ROMs.[22]

//EX9E	KeyOp	if (key() == Vx)	Skips the next instruction if the key stored in VX is pressed (usually the next instruction is a jump to skip a code block).[22]
//EXA1	KeyOp	if (key() != Vx)	Skips the next instruction if the key stored in VX is not pressed (usually the next instruction is a jump to skip a code block).[22]
//FX07	Timer	Vx = get_delay()	Sets VX to the value of the delay timer.[22]
//FX0A	KeyOp	Vx = get_key()	A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event).[22]
//FX15	Timer	delay_timer(Vx)	Sets the delay timer to VX.[22]
//FX18	Sound	sound_timer(Vx)	Sets the sound timer to VX.[22]
//FX1E	MEM	I += Vx	Adds VX to I. VF is not affected.[c][22]
//FX29	MEM	I = sprite_addr[Vx]	Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.[22]
//FX33	BCD	
//set_BCD(Vx)
//*(I+0) = BCD(3);
//*(I+1) = BCD(2);
//*(I+2) = BCD(1);
//Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.[22]
//FX55	MEM	reg_dump(Vx, &I)	Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.[d][22]
//FX65	MEM	reg_load(Vx, &I)	Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified.[d][22]

void JChip8::execute_instruction(instruction& instr)
{
    switch (instr.opcode >> 12)
    {
        case 0x00:
            if (instr.NN == 0xE0)
            {
                std::cout << "Clear screen";
                clear_graphics_buffer();
            }
            else if (instr.NN == 0xEE)
            {
                std::cout << "Return from subroutine";
                pc = stack[--sp];
            }
            break;

        case 0x01:
            std::cout << "Jump to address NNN";
            pc = instr.NNN;
            break;

        case 0x02:
            std::cout << "Call subroutine at NNN";
            stack[sp++] = pc;
            pc = instr.NNN;
            break;

        case 0x03:
            std::cout << "If Vx == NN, skip the next instruction";
            if (V[instr.X] == instr.NN) pc += 2;
            break;

        case 0x04:
            std::cout << "If Vx != NN, skip the next instruction";
            if (V[instr.X] != instr.NN) pc += 2;
            break;

        case 0x05:
            std::cout << "5XY0 if (Vx == Vy) skip the next instruction";
            if (V[instr.X] == V[instr.Y])
                pc += 2;
            break;

        case 0x06:
            std::cout << "6XNN	Sets VX to NN.";
            V[instr.X] = instr.NN;
            break;

        case 0x07:
            std::cout << "7XNN	Vx += NN Adds NN to VX (carry flag is not changed)";
            V[instr.X] += instr.NN;
            break;

        case 0x08:
            switch (instr.N)            
            {
                case 0x00:
                    std::cout << "8XY0 Vx = Vy Sets VX to the value of VY.";
                    V[instr.X] = V[instr.Y];
                    break;

                case 0x01:
                    std::cout << "8XY1 Vx |= Vy sets VX to VX or VY. (bitwise OR operation).";
                    V[instr.X] |= V[instr.Y];
                    break;

                case 0x02:
                    std::cout << "8XY2 Vx &= Vy sets VX to VX and VY. (bitwise AND operation).";
                    V[instr.X] &= V[instr.Y];
                    break;

                case 0x03:
                    std::cout << "8XY3[a] Vx ^= Vy sets VX to VX xor VY.";
                    V[instr.X] ^= V[instr.Y];
                    break;

                case 0x04:
                {
                    std::cout << "8XY4 Vx += Vy Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.";
                    uint16 sum = V[instr.X] + V[instr.Y];
                    if (sum > 0xFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    V[instr.X] = sum & 0xFF;
                    break;
                }

                case 0x05:
                    break;

                case 0x06:
                    break;

                case 0x07:
                    break;

                case 0x0E:
                    break;

                default:
                    break;
            }
            
            //8XY5	Math	Vx -= Vy	VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not).[22]
            //8XY6[a]	BitOp	Vx >>= 1	Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.[b][22]
            //8XY7[a]	Math	Vx = Vy - Vx	Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX).[22]
            //8XYE[a]	BitOp	Vx <<= 1	Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset.[b][22]
            break;

        case 0x09:
            std::cout << "9XY0 if (Vx != Vy) skips the next instruction";
            if (V[instr.X] != V[instr.Y]) pc += 2;
            break;

        case 0x0A:
            std::cout << "ANNN	Sets I to NNN.";
            I = instr.NNN;
            break;

        case 0x0B:
            std::cout << "BNNN	PC = V0 + NNN Jumps to the address NNN plus V0.";
            pc = instr.NNN + V[0];
            break;

        case 0x0C:
            std::cout << "CXNN	Vx = rand() & NN Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.";
            break;

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
            std::cout << "Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.";
            V[0xF] = 0;
            _draw_flag = true;
            uint8 height = instr.N;

            for (uint8 i = 0; i < height; ++i)
            {
                uint8 sprite = memory[I + i];
                uint8 row = (V[instr.Y] + i) % GRAPHICS_HEIGHT;

                for (int8 j = 0; j < 8; ++j)
                {
                    uint8 bit = (sprite & 0x80) >> 7;
                    uint8 col = (V[instr.X] + j) % GRAPHICS_WIDTH;
                    uint16 offset = row * GRAPHICS_WIDTH + col;

                    if (bit == 1)
                    {
                        if (graphics[offset] != 0)
                        {
                            graphics[offset] = 0;
                            V[0xF] = 1;
                        }
                        else
                        {
                            graphics[offset] = 1;
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

            }
            else if (instr.NN == 0xA1)
            {

            }
            break;

        case 0x0F:
            switch (instr.NN)
            {
                case 0x07:
                    std::cout << "Setting Vx to value of delay timer";
                    V[instr.X] = delay_timer;
                    break;

                case 0x0A:
                    std::cout << "Awaiting key press, then storing in Vx (blocking, will not continue until keypress occurs)";
                    break;

                case 0x15:
                    std::cout << "Sets the delay timer to Vx";
                    delay_timer = V[instr.X];
                    break;

                case 0x18:
                    std::cout << "Sets the sound timer to Vx";
                    sound_timer = V[instr.X];
                    break;

                case 0x1E:
                    std::cout << "Adds Vx to I.  Vf (carry flag) unaffected";
                    I += V[instr.X];
                    break;

                case 0x29:
                    std::cout << "Sets I to the location of the sprite for the character in Vx";
                    break;

                case 0x33:
                    std::cout << "Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.";
                    break;

                case 0x55:
                {
                    std::cout << "Stores from V0 to VX (inclusive) into memory, starting at address I using offsets;  I is unchanged";
                    uint8 len = instr.X;
                    for (uint8 i = 0; i <= len; ++i)
                    {
                        memory[I + i] = V[i];
                    }
                    break;
                }

                case 0x65:
                    std::cout << "Fills from V0 to VX (inclusive) from memory, starting at address I using offsets;  I is unchanged";
                    uint8 len = instr.X;
                    for (uint8 i = 0; i <= len; ++i)
                    {
                        V[i] = memory[I + i];
                    }
                    break;

                default:
                    std::cout << "Unimplemented 0x0F instruction";
                    break;
            }
            break;

        // --------------------------------------------------
        //                  Draw Instruction
        // --------------------------------------------------

        default:
            std::cout << "Unimplemented instruction";
            break;
    }
    std::cout << "]\n";
}

void JChip8::load_game(const ROM& rom)
{
    std::ifstream file(rom.filepath, std::ios::binary);
    if (!file) throw std::runtime_error("Could not open file");

    if (rom.size > (MEMORY_SIZE - 0x200)) throw std::runtime_error("File too large to fit in memory");

    for (size_t i = 0; i < rom.size; ++i)
    {
        memory[0x200 + i] = file.get();
    }
}

void JChip8::reset_draw_flag() { _draw_flag = false; }

void JChip8::load_fontset()
{
    uint8 fontset[80] =
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

void JChip8::clear_graphics_buffer()
{
    memset(graphics, false, sizeof(graphics));
    _draw_flag = true;
}

