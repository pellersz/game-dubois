#include "cartridge.h"
#include "controller.h"
#include "gameboy.h"
#include "mem.h"
#include "ppu.h"
#include "scheduler.h"
#include "screen.h"
#include <memory>

// TODO: be a bit more professional (declare when values are const consts)
// TODO: have the constantly usable registers and such as class members, rather than asking the memory each time

int main() {
    Memory mem;
    Controller cont(mem);
    Screen screen;
    Ppu ppu(mem, screen);
    Scheduler sc(mem, cont, ppu, screen);
    Cpu cpu(mem, sc);
    sc.init(std::make_shared<Cpu>(cpu));

    GameBoy gameboy(mem, cont, sc, cpu, ppu, screen); 
    Cartridge cartridge("./src/tetris.gb");
    gameboy.load(std::make_shared<Cartridge>(cartridge));

    gameboy.run();
}
