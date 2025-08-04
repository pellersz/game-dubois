#include "scheduler.h"
#include "cpu.h"
#include "mem.h"
#include "types.h"
#include <memory>

Scheduler::Scheduler(Memory& memory) : memory(memory) { schedule.push(ProcessStart(time, CPU_EXEC)); }

void Scheduler::init(std::shared_ptr<Cpu> cpu_ptr) { cpu = cpu_ptr; }

void Scheduler::push(u8 duration, u8 process) { schedule.push(ProcessStart(time + duration, process)); }

void Scheduler::pop() 
{
    static byte last_val = memory[Memory::DIVIDER_REGISTER];
    switch (schedule.top().second) {
        case CPU_EXEC: { cpu->executeNext() ; break; }
        case UPDATE_DIV: 
        {
            byte& divider = memory[Memory::DIVIDER_REGISTER];
            push((u8) 256, UPDATE_DIV);
            if (divider == last_val) 
            {
                last_val = (++divider);
                break;
            }
            last_val = (divider = 0);
            break;
        } 
        case UPDATE_TIMA: 
        {
            const u8 vals[] = { (u8) 1024, (u8) 16, (u8) 64, (u8) 256 };
            u8 val = vals[memory[Memory::TIMER_CONTROL] & 0b0011];
            push(val, UPDATE_TIMA);
            if (memory[Memory::TIMER_CONTROL] & 0b0100)
            {
                byte& tima = ++memory[Memory::TIMER_COUNTER];
                if (!tima) 
                {
                    tima = memory[Memory::TIMER_MODULO];
                    memory[Memory::INTERRUPT_FLAG] |= 0b0100;
                }
            }
        }
    };
    schedule.pop(); 
}

void Scheduler::run() 
{
    while (true) {
        if (schedule.top().first == time)
            pop();
        else
            tick();
    }
}

void Scheduler::stop() {}

void Scheduler::tick() 
{
    ++time;
}

