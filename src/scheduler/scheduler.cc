#include "scheduler.h"
#include "mem.h"

Scheduler::Scheduler(Cpu& cpu, Memory& memory) : cpu(cpu), memory(memory) {}

void Scheduler::tick(Ticks ticks) { 
    time += (unsigned long long) ticks;
}

