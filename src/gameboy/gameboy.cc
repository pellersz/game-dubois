#include "gameboy.h"
#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include "scheduler.h"

GameBoy::GameBoy(Memory& memory, Controller& controller, Scheduler& scheduler, Cpu& cpu) : 
    memory(memory), 
    controller(controller), 
    scheduler(scheduler),
    cpu(cpu)
{ }

void GameBoy::run() {
    scheduler.run();
}
