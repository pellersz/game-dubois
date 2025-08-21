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
    GameBoy();
    
    void load(std::shared_ptr<Cartridge>);
    void unload();
    
    void run(bool);

private:
    Memory memory;
    Controller controller = Controller(memory);
    Screen screen;
    Ppu ppu = Ppu(memory, screen);
    Scheduler scheduler = Scheduler(memory, controller, ppu, screen);
    Cpu cpu = Cpu(memory, scheduler);
    std::shared_ptr<Cartridge> cartridge = nullptr;
};

#endif
