#include "gameboy.h"
#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include "scheduler.h"
#include <iostream>

GameBoy::GameBoy() : 
    memory(), 
    controller(memory), 
    scheduler(memory, controller),
    cpu(memory, scheduler)
{
    std::cout << "yup" << std::endl;
}

void GameBoy::run() {
    scheduler.run();
}
