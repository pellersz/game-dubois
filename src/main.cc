#include "cartridge.h"
#include "gameboy.h"
#include <iostream>

int main(int argc, char** argv) 
{
    //for(int i = 0; i < 0x100; ++i)
    //{
    //    std::stringstream s;
    //    s << "tests/v2/" << std::hex << std::setfill('0') << std::setw(2) << i << ".json";
    //    if (i == 0x27 || i == 0xe8 || i == 0xf8) 
    //        continue;
    //    std::cout << std::endl << s.str() << std::endl;
    //    cpu.test(s.str());
    //}
    if (argc == 1)
    {
        std::cout << "Need at least one argument";
        return 1;
    }

    GameBoy gameboy;
    gameboy.load(std::make_shared<Cartridge>(argv[1]));
    if (argc == 2) 
        gameboy.run(false);
    else 
        gameboy.run(true);
}
