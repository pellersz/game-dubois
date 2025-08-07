#include "controller.h"
#include "gameboy.h"
#include "mem.h"
#include "scheduler.h"
#include <memory>

// TODO: be a bit more professional (declare when values are const consts)
// TODO: have the constantly usable registers and such as class members, rather than asking the memory each time

int main() {
    Memory mem;
    Controller cont(mem);
    Scheduler sc(mem, cont);
    Cpu cpu(mem, sc);
    sc.init(std::make_shared<Cpu>(cpu));
    GameBoy gameboy(mem, cont, sc, cpu); 
    gameboy.run();
}
