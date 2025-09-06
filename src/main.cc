#include "cartridge.h"
#include "gameboy.h"
#include <iostream>

// TODO: make it so that you can alt-tab
// TODO: forward declarations
int main(int argc, char** argv) 
{
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
