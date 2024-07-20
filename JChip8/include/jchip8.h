#ifndef JUMI_JCHIP8_H
#define JUMI_JCHIP8_H
#include <string>

//0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
//0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
//0x200-0xFFF - Program ROM and work RAM

#define MAX_MEMORY 4096

using opcode = unsigned short;

class JChip8
{
public:
    unsigned char memory[MAX_MEMORY];
    unsigned char V[16];
    unsigned short I;
    unsigned short pc;
    unsigned char gfx[64 * 32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned short sp;
    unsigned char key[16];

    JChip8();

    opcode fetch_opcode() const;
    void initialize();
    void load_game(const std::string& game_name);
    void emulate_cycle();
    void set_keys();

private:
};

#endif
