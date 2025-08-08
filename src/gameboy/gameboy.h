#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"
#include "scheduler.h"
#include "cartridge.h"
#include "screen.h"
#include <memory>

class GameBoy {
public:
    GameBoy(Memory&, Controller&, Scheduler&, Cpu&, Ppu&, Screen&);
    
    void load(std::shared_ptr<Cartridge>);
    void unload();
    
    void run();

private:
    Memory& memory;
    Controller& controller;
    Scheduler& scheduler;
    Cpu& cpu;
    Ppu& ppu;
    Screen& screen;
    std::shared_ptr<Cartridge> cartridge = nullptr;
};

#endif
