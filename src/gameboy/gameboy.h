#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include "scheduler.h"

class GameBoy {
public:
    GameBoy();
    void run();

private:
    Memory memory;
    Controller controller;
    Scheduler scheduler;
    Cpu cpu;
};

#endif
