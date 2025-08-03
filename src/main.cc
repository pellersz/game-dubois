#include "cpu.h"
#include "mem.h"
#include "scheduler.h"
#include <iostream>
#include <memory>

int main() {
    Memory memory;
    Scheduler scheduler(memory);
    Cpu cpu(memory, scheduler);
    auto cpu_ptr = std::make_shared<Cpu>(cpu);
    scheduler.init(cpu_ptr);
    memory[1] = 1;
    std::cout << "I live " << (int)memory[1] << " " << " yes";
    scheduler.run();
}
