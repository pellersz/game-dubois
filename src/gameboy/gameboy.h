#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include "scheduler.h"
#include "cartridge.h"
#include <memory>

class GameBoy {
public:
    GameBoy(Memory&, Controller&, Scheduler&, Cpu&);
    
    void load(std::shared_ptr<Cartridge>);
    void unload();
    
    bool powerUpSequence();
    void run();

private:
    Memory& memory;
    Controller& controller;
    Scheduler& scheduler;
    Cpu& cpu;
    std::shared_ptr<Cartridge> cartridge = nullptr;
};

#endif
