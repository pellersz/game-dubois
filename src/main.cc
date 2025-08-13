#include "cartridge.h"
#include "controller.h"
#include "gameboy.h"
#include "mem.h"
#include "ppu.h"
#include "scheduler.h"
#include "screen.h"
#include <memory>

// TODO: be a bit more professional (declare when values are const consts)
// TODO: investigate cpu shared_ptr having a brain split when using it the right way

int main() {
    Memory mem;
    Controller cont(mem);
    Screen screen;
    Ppu ppu(mem, screen);
    Scheduler sc(mem, cont, ppu, screen);
    Cpu cpu(mem, sc);
    sc.init(std::make_shared<Cpu>(cpu));

    GameBoy gameboy(mem, cont, sc, cpu, ppu, screen); 
    gameboy.load(std::make_shared<Cartridge>("./src/tetris.gb"));

    gameboy.run();
}
