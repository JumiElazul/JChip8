#include "j_assembler.h"
#include <fstream>
#include <string>

void j_assembler::parse_input_file(const char* filepath)
{
    std::ifstream file(filepath);

    if (!file) 
    {
        std::string message = std::string("File with filename: ") + filepath + " does not exist\n";
        throw std::runtime_error(message.c_str());
    }


}
