#include "scheduler.h"
#include "cpu.h"
#include "mem.h"
#include <memory>

Scheduler::Scheduler(Memory& memory) : memory(memory) {}

void Scheduler::init(std::shared_ptr<Cpu> cpu_ptr) { cpu = cpu_ptr; }

void Scheduler::push(u8 duration, u8 process) { schedule.push(ProcessStart(time + duration, process)); }

void Scheduler::pop() 
{
    switch (schedule.top().second) {
        case CPU_EXEC: { cpu->executeNext() ; break;}
        case UPDATE_TIME: { ; break;}
    };
    schedule.pop(); 
}

void Scheduler::start() {
    
}

void Scheduler::stop() {

}

void Scheduler::tick() { 
    ++time;
}

