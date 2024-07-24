#include "jchip8.h"
#include <fstream>
#include <iostream>
#include <iomanip>

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
    , state{ emulator_state::running }
    , _instruction_history{ }
    , _instruction_pointer{ 0 }
{
    load_fontset();
}

void JChip8::emulate_cycle()
{
    instruction instr = fetch_instruction();
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

instruction JChip8::fetch_instruction()
{
    instruction instr;
    instr.opcode = memory[pc] << 8 | memory[pc + 1];
    instr.NNN = instr.opcode & 0x0FFF;
    instr.NN  = instr.opcode & 0x00FF;
    instr.N   = instr.opcode & 0x000F;
    instr.X   = (instr.opcode & 0x0F00) >> 8;
    instr.Y   = (instr.opcode & 0x00F0) >> 4;

    update_instruction_history(instr);
    pc += 2;

    return instr;
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
#ifndef NDEBUG
    std::pair<unsigned short, instruction>& last_instruction = _instruction_history[_instruction_pointer - 1];
    std::cout << "Address: [0x" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << last_instruction.first
              << "]   Instruction: [0x" << std::setw(4) << std::setfill('0') << last_instruction.second.opcode
              << "]   Description: [";
#endif

    switch ((instr.opcode >> 12) & 0x0F)
    {
        case 0x00:
            if (instr.NN == 0xE0)
            {
                std::cout << "Clear screen";
                clear_screen();
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
                    std::cout << "8XY4 Vx += Vy Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.";
                    unsigned short sum = V[instr.X] + V[instr.Y];
                    if (sum > 0xFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    V[instr.X] = sum & 0xFF;
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
            std::cout << "ANNN	Sets I to the address NNN.";
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
            std::cout << "Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.";
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
    memset(graphics, false, sizeof(graphics));
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

void JChip8::update_instruction_history(instruction& instr)
{
    std::pair<unsigned short, instruction> current_instr;
    current_instr.first = pc;
    current_instr.second = instr;

    _instruction_history[_instruction_pointer++] = current_instr;

    if (_instruction_pointer >= INSTRUCTION_MEMORY_SIZE)
        _instruction_pointer = 0;
}

