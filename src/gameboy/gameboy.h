#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "apu.h"
#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"
#include "scheduler.h"
#include "cartridge.h"
#include "screen.h"
#include "speaker.h"
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
    Speaker speaker;
    Apu apu = Apu(memory, speaker);
    Scheduler scheduler = Scheduler(memory, controller, ppu, screen, apu);
    Cpu cpu = Cpu(memory, apu);
    std::shared_ptr<Cartridge> pCartridge = nullptr;
};

#endif
