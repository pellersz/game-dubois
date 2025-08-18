#include "cartridge.h"
#include "controller.h"
#include "gameboy.h"
#include "mem.h"
#include "ppu.h"
#include "scheduler.h"
#include "screen.h"
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>

// TODO: be a bit more professional (declare when values are const consts)
// TODO: investigate cpu shared_ptr having a brain split when using it the right way

int main(int argc, char** argv) 
{
    Memory mem;
    Controller cont(mem);
    Screen screen;
    Ppu ppu(mem, screen);
    Scheduler sc(mem, cont, ppu, screen);
    Cpu cpu(mem, sc);
    sc.init(std::make_shared<Cpu>(cpu));

    //for(int i = 0; i < 0x100; ++i)
    //{
    //    std::stringstream s;
    //    s << "tests/v2/" << std::hex << std::setfill('0') << std::setw(2) << i << ".json";
    //    if (i == 0x27 || i == 0xe8 || i == 0xf8) 
    //        continue;
    //    std::cout << std::endl << s.str() << std::endl;
    //    cpu.test(s.str());
    //}
    GameBoy gameboy(mem, cont, sc, cpu, ppu, screen); 
    gameboy.load(std::make_shared<Cartridge>("./src/11-op a,(hl).gb"));

    if (argc == 1) 
        gameboy.run(false);
    else 
        gameboy.run(true);
}
