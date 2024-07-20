#include "jchip8.h"
#include <string>
#include <stdexcept>

JChip8::JChip8()
    : memory()
    , V()
    , I()
    , pc()
    , gfx()
    , delay_timer()
    , sound_timer()
    , stack()
    , sp()
    , key()
{

}

opcode JChip8::fetch_opcode() const
{

}

void JChip8::initialize()
{

}

void JChip8::load_game(const std::string& game_name)
{
}

void JChip8::emulate_cycle()
{

}

void JChip8::set_keys()
{

}
