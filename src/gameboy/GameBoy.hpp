#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "Apu.hpp"
#include "Controller.hpp"
#include "Cpu.hpp"
#include "Memory.hpp"
#include "Ppu.hpp"
#include "Scheduler.hpp"
#include "Cartridge.hpp"
#include "Screen.hpp"
#include "Speaker.hpp"
#include <memory>

class GameBoy {
  public:
    GameBoy();
    
    void load(std::shared_ptr<Cartridge>);
    void unload();
    void loadRam(std::string);
    
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
