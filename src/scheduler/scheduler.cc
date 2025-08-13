#include "scheduler.h"
#include "GLFW/glfw3.h"
#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"
#include "screen.h"
#include "types.h"
#include <iostream>
#include <memory>
#include <chrono>

using namespace std::chrono;

const unsigned Scheduler::SYSTEM_CLOCKS_PER_DOT = sysconf(_SC_CLK_TCK) / MASTER_CLOCK_FREQUENCY;

Scheduler::Scheduler(Memory& memory, Controller& controller, Ppu& ppu, Screen& screen) : 
    memory(memory), 
    controller(controller), 
    ppu(ppu),
    screen(screen)
{ 
    last_div = memory[Memory::DIVIDER_REGISTER];

    last_boot_rom = memory[Memory::BOOT_ROM_MAPPING];

    schedule.push(ProcessStart(time, CPU_EXEC)); 
    schedule.push(ProcessStart(time, UPDATE_DIV));
    schedule.push(ProcessStart(time, UPDATE_TIMA));
    schedule.push(ProcessStart(time, VBLANK_START));
    schedule.push(ProcessStart(time, LYC_LY_CMP));
    schedule.push(ProcessStart(time, HANDLE_CONTROL));
}

void Scheduler::init(std::shared_ptr<Cpu> cpu_ptr) { cpu = cpu_ptr; }

void Scheduler::push(unsigned short duration, Process process) { schedule.push(ProcessStart(time + duration, process)); }

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
            {
                last_boot_rom = memory[Memory::BOOT_ROM_MAPPING];
                go_next = false;
            }

            break; 
        }
        case UPDATE_DIV: 
        {
            byte& divider = memory[Memory::DIVIDER_REGISTER];
            push((unsigned short) 256, UPDATE_DIV);
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
            const unsigned short vals[] = { 1024, 16, 64, 256 };
            short val = vals[memory[Memory::TIMER_CONTROL] & 0b0011];
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
        case VBLANK_START: 
        {
            //static int counter = 0;
            //if (!(counter % 60)) std::cout << " " << (int) counter / 60 << std::endl;
            //
            memory[Memory::INTERRUPT_FLAG] |= 0b0001; 
            screen.updateFrame();
            ++memory[Memory::LCD_Y];

            push(456 * Ppu::TIME_UNIT, VBLANK);
            while(next_dot_time > std::chrono::steady_clock::now()) {}
            next_dot_time += SYSTEM_CLOCKS_PER_DOTT;
            break;
        }
        case VBLANK: 
        {
            byte& lcd_y = memory[Memory::LCD_Y];
            if (++lcd_y < 154) 
            {
                push(456 * Ppu::TIME_UNIT, VBLANK);
                break;
            }
            lcd_y = 0;
            push(456 * Ppu::TIME_UNIT, OAM_SCAN);
            break;
        }
        case LYC_LY_CMP: 
        { 
            push(4, LYC_LY_CMP);
            if (memory[Memory::LCD_Y] == memory[Memory::LCD_CONTROL]) 
            {
                memory[Memory::LCD_STAT] |= 0b0100;
                break;
            }
            memory[Memory::LCD_STAT] &= 0b1011;
            break;
        }
        case OAM_SCAN: 
        { 
            byte& ly = memory[Memory::LCD_Y];
            ppu.oamScan();
            push(80 * Ppu::TIME_UNIT, DRAW_PIXELS);
            break;
        }
        case DRAW_PIXELS:
        {
            ppu.drawLine();
            push(172 * Ppu::TIME_UNIT, HBLANK);
            ++memory[Memory::LCD_Y];
            break;
        }
        case HBLANK:
        {
            ppu.hBlank();
            if (memory[Memory::LCD_Y] < 143) 
            {
                push(87 * Ppu::TIME_UNIT, OAM_SCAN);
                break;
            }

            push(87 * Ppu::TIME_UNIT, VBLANK_START);
            break;
        }
        case HANDLE_CONTROL: 
        {
            glfwPollEvents();
            if(glfwGetKey(screen.getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
                controller.buttonPressed(Controller::RIGHT_PRESSED);
            if(glfwGetKey(screen.getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
                controller.buttonPressed(Controller::LEFT_PRESSED);
            if(glfwGetKey(screen.getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS)
                controller.buttonPressed(Controller::DOWN_PRESSED);
            if(glfwGetKey(screen.getWindow(), GLFW_KEY_UP) == GLFW_PRESS)
                controller.buttonPressed(Controller::UP_PRESSED);
            if(glfwGetKey(screen.getWindow(), GLFW_KEY_Z) == GLFW_PRESS)
                controller.buttonPressed(Controller::A_PRESSED);
            if(glfwGetKey(screen.getWindow(), GLFW_KEY_X) == GLFW_PRESS)
                controller.buttonPressed(Controller::B_PRESSED);
            if(glfwGetKey(screen.getWindow(), GLFW_KEY_ENTER) == GLFW_PRESS)
                controller.buttonPressed(Controller::START_PRESSED);
            if(glfwGetKey(screen.getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                controller.buttonPressed(Controller::SELECT_PRESSED);
            
            push(100, HANDLE_CONTROL);
            break;
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
    next_dot_time = steady_clock::now() + SYSTEM_CLOCKS_PER_DOTT;

    while (go_next && !glfwWindowShouldClose(screen.getWindow())) {
        if (schedule.top().first <= time)
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

void Scheduler::tick() { time += 4; }

