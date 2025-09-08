#include "cartridge.h"
#include "gameboy.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

// TODO: make it so that you can alt-tab
int main(int argc, char** argv) 
{
    bool debug = false;
    std::string cartridge_path = "";
    std::string ram_path = "";

    // this is not the best, but the problem that it solves is simple enough for it to be used like this
    for (int i = 1; i < argc;)
    {
        if (!strcmp(argv[i], "-d"))
        {
            debug = true;
            ++i;
        }
        else if (!strcmp(argv[i], "-c"))
        {
            if (argc <= i + 1)
            {
                ++i;
                continue;
            }
            cartridge_path = argv[i + 1];
            i += 2;
        }
        else if (!strcmp(argv[i], "-r"))
        {
            if (argc <= i + 1)
            {
                std::cout << "No specified path for the ram" << std::endl;
                break;
            }
            ram_path = argv[i + 1];
            i += 2;
        }
    }

    if (cartridge_path == "")
    {
        std::cout << "No cartridge path specified" << std::endl;
        exit(1);
    }

    std::shared_ptr<Cartridge> p_cartridge = std::make_shared<Cartridge>(cartridge_path);
    if (ram_path != "")
        p_cartridge->loadRam(ram_path);


    GameBoy gameboy;
    gameboy.load(p_cartridge);
    
    gameboy.run(debug);
}

