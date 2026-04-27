#include "Cartridge.hpp"
#include "GameBoy.hpp"
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

// TODO: make it so that you can alt-tab
int main(int argc, char** argv) 
{
    bool debug = false;
    std::string cartridgePath = "";
    std::string ramPath = "";

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
            cartridgePath = argv[i + 1];
            i += 2;
        }
        else if (!strcmp(argv[i], "-r"))
        {
            if (argc <= i + 1)
            {
                std::cout << "No specified path for the ram" << std::endl;
                exit(1);
            }
            ramPath = argv[i + 1];
            i += 2;
        }
        else 
        {
            std::cout << "No such option " << argv[i] << std::endl;
            exit(1);
        }
    }

    if (cartridgePath == "")
    {
        std::cout << "No cartridge path specified" << std::endl;
        exit(1);
    }

    std::shared_ptr<Cartridge> pCartridge = std::make_shared<Cartridge>(cartridgePath);
    if (ramPath != "")
        pCartridge->loadRam(ramPath);

    GameBoy gameboy;
    gameboy.load(pCartridge);
    
    gameboy.run(debug);
}

