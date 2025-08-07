#include "scheduler.h"
#include "cpu.h"
#include "mem.h"
#include "types.h"
#include <iostream>
#include <memory>

Scheduler::Scheduler(Memory& memory, Controller& controller) : memory(memory), controller(controller) 
{ 
    last_div = memory[Memory::DIVIDER_REGISTER];
    last_boot_rom = memory[Memory::BOOT_ROM_MAPPING];
    schedule.push(ProcessStart(time, CPU_EXEC)); 
}


void Scheduler::init(std::shared_ptr<Cpu> cpu_ptr) { cpu = cpu_ptr; }

void Scheduler::push(u8 duration, Process process) { schedule.push(ProcessStart(time + duration, process)); }

bool Scheduler::pop() 
{
    bool go_next = true;

    switch (schedule.top().second) {
        case CPU_EXEC:
        {
            // since only the cpu cares about controller input, it only should update before it
            controller.updatePressed();
            cpu->executeNext();
            if ((last_boot_rom != memory[Memory::BOOT_ROM_MAPPING]) && 
                    memory[Memory::BOOT_ROM_MAPPING] == 1)
                go_next = false;

            break; 
        }
        case UPDATE_DIV: 
        {
            byte& divider = memory[Memory::DIVIDER_REGISTER];
            push((u8) 256, UPDATE_DIV);
            if (divider == last_div) 
            {
                last_div = ++divider;
                break;
            }
            last_div = (divider = 0);
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
            break;
        }
        case VBLANK: { memory[Memory::INTERRUPT_FLAG] |= 0b0001; }
        case LYC_LY_CMP: 
        { 
            push(1, LYC_LY_CMP);
            if (memory[Memory::LCD_Y] == memory[Memory::LCD_CONTROL]) 
            {
                memory[Memory::LCD_STAT] |= 0b0100;

                break;
            }
            memory[Memory::LCD_STAT] &= 0b1011;
        }
    };
    schedule.pop(); 
    return go_next;
}

void Scheduler::run() 
{
    if (cpu == nullptr) {
        std::cout << "Initialize the cpu, bozo";
        return;
    }

    bool go_next = true;
    while (go_next) {
        if (schedule.top().first == time)
            go_next = pop();
        else
            tick();
    }
}

void Scheduler::stop() {}

// TODO: possibly make sure these work as they should
void Scheduler::statInterruptCheck() 
{
    bool tmp = statInterruptLine;
    byte& stat_register = memory[Memory::LCD_STAT];
    statInterruptLine = ((stat_register & 0b01000000) && (stat_register & 0b0100))           || 
                        ((stat_register & 0b00100000) && ((stat_register & 0b0011) == 0b10)) ||
                        ((stat_register & 0b00010000) && ((stat_register & 0b0011) == 0b01)) ||
                        ((stat_register & 0b00001000) && ((stat_register & 0b0011) == 0b00));
    if (tmp < statInterruptLine) 
        memory[Memory::INTERRUPT_FLAG] |= 0b0010;
}

void Scheduler::tick() { ++time; }

