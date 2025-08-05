#include "controller.h"
#include "gameboy.h"
#include "mem.h"
#include "scheduler.h"
#include <memory>

// TODO: be a bit more professional (declare when values are const consts)

int main() {
    Memory mem;
    Controller cont(mem);
    Scheduler sc(mem, cont);
    Cpu cpu(mem, sc);
    sc.init(std::make_shared<Cpu>(cpu));
    GameBoy gameboy(mem, cont, sc, cpu); 
    gameboy.run();
}
