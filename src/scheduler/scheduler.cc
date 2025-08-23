#include "scheduler.h"
#include "GLFW/glfw3.h"
#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include "ppu.h"
#include "screen.h"
#include "types.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <chrono>
#include <unistd.h>

using namespace std::chrono;

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

void Scheduler::init(std::shared_ptr<Cpu> cpu_ptr) { p_cpu = cpu_ptr; }

void Scheduler::push(unsigned short duration, Process process) { schedule.push(ProcessStart(time + duration, process)); }

// This is acceptable because there are only 6 events at a time
void Scheduler::replace(Process process, unsigned short duration) 
{
    std::priority_queue<
        ProcessStart,
        std::vector<ProcessStart>,
        ProcessGreater 
    > new_schedule;

    new_schedule.push(ProcessStart {time + duration, process});
    while (!schedule.empty()) 
    {
        if (schedule.top().second != process)
            new_schedule.push(schedule.top());
        schedule.pop();
    }

    schedule = new_schedule;
}

bool Scheduler::pop() 
{
    bool go_next = true;

    switch (schedule.top().second) 
    {
        case CPU_EXEC:
        {
            // since only the cpu cares about controller input, it only should update before it
            controller.updatePressed();
            p_cpu->executeNext();

            if ((last_boot_rom != memory[Memory::BOOT_ROM_MAPPING]) && 
                    (memory[Memory::BOOT_ROM_MAPPING] == 1))
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
            short val = TIMA_PERIODS[memory[Memory::TIMER_CONTROL] & 0b0011];
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
            memory[Memory::INTERRUPT_FLAG] |= 0b0001; 
            screen.updateFrame();
            ppu.resetWindowY();
            ++memory[Memory::LCD_Y];

            push(456 * Ppu::TIME_UNIT, VBLANK);
            //while(next_dot_time > std::chrono::steady_clock::now()) {}
            //next_dot_time = steady_clock::now() + SYSTEM_CLOCKS_PER_DOT;
 
            byte& lcd_stat = memory[Memory::LCD_STAT];
            lcd_stat = (lcd_stat & 0b11111100) + 0b01;
            statInterruptCheck();

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
            byte ly = memory[Memory::LCD_Y];
            push(4, LYC_LY_CMP);
            if (ly == memory[Memory::LY_COMPARE]) 
            {
                if (last_ly != ly) 
                {
                    memory[Memory::LCD_STAT] |= 0b0100;
                    statInterruptCheck();
                    break;
                }
                last_ly = ly;
            }
            memory[Memory::LCD_STAT] &= 0b11111011;
            break;
        }
        case OAM_SCAN: 
        { 
            ppu.oamScan();

            byte& lcd_stat = memory[Memory::LCD_STAT];
            lcd_stat = (lcd_stat & 0b11111100) + 0b10;
            statInterruptCheck();

            push(80 * Ppu::TIME_UNIT, DRAW_PIXELS);
            break;
        }
        case DRAW_PIXELS:
        {
            ppu.drawLine();
            push(172 * Ppu::TIME_UNIT, HBLANK);
            ++memory[Memory::LCD_Y];

            byte& lcd_stat = memory[Memory::LCD_STAT];
            lcd_stat = (lcd_stat & 0b11111100) + 0b11;
            statInterruptCheck();

            break;
        }
        case HBLANK:
        {
            ppu.hBlank();
            
            byte& lcd_stat = memory[Memory::LCD_STAT];
            lcd_stat = (lcd_stat & 0b11111100) + 0b00;
            statInterruptCheck();

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
            GLFWwindow *window = screen.getWindow();
            glfwPollEvents();
            if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                controller.buttonPressed(Controller::RIGHT);
            else 
                controller.buttonReleased(Controller::RIGHT);

            if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                controller.buttonPressed(Controller::LEFT);
            else 
                controller.buttonReleased(Controller::LEFT);

            if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                controller.buttonPressed(Controller::DOWN);
            else 
                controller.buttonReleased(Controller::DOWN);

            if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                controller.buttonPressed(Controller::UP);
            else 
                controller.buttonReleased(Controller::UP);

            if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
                controller.buttonPressed(Controller::A);
            else 
                controller.buttonReleased(Controller::A);

            if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
                controller.buttonPressed(Controller::B);
            else 
                controller.buttonReleased(Controller::B);

            if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
                controller.buttonPressed(Controller::START);
            else 
                controller.buttonReleased(Controller::START);

            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                controller.buttonPressed(Controller::SELECT);
            else 
                controller.buttonReleased(Controller::SELECT);
            
            push(1000, HANDLE_CONTROL);

            break;
        }
    };

    schedule.pop(); 
    return go_next;
}

void Scheduler::run() 
{ 
    if (p_cpu == nullptr) 
    {
        std::cout << "Initialize the cpu, bozo";
        return;
    } 

    bool go_next = true;
    next_dot_time = steady_clock::now() + SYSTEM_CLOCKS_PER_DOT;

    while (go_next && !glfwWindowShouldClose(screen.getWindow())) 
    {
        if (schedule.top().first <= time)
            go_next = pop();
        else
            tick();
    }
}

void Scheduler::handleDebugStop(bool& mode, int& stop_at) 
{
    char action;
    bool done = false;
    while (!done)
    {
        done = true;
        std::cin >> action;
        switch (action) {
            case 'n': { stop_at = -1; break; }
            case 'c': 
            { 
                if (!mode)
                    stop_at = 0xffff; 
                break; 
            }
            case 'b': 
            {
                mode = false;
                std::cin >> std::hex >> stop_at; 
                break; 
            }
            case 'f':
            {
                mode = true;
                std::cin >> std::hex >> stop_at; 
                break; 
            }
            case 'm': 
            {
                word addr;
                std::cin >> std::hex >> addr; 
                std::cout << std::hex << (int) memory[addr] << std::endl;
                done = false;
                break; 
            }
            case 'p':
            {
                ppu.printUsedTiles();
                done = false;
                break;
            }
            case 'h': 
            {
                done = false;
                std::cout                                               << 
                    "n: step once\n"                                    <<
                    "b <offs>: break when pc is at <offs>\n"            <<
                    "f <byte>: break when memory[pc] == <byte>\n"       <<
                    "m <offs>: print byte at memory offset <offs>\n"    <<
                    "p: print all 4 configurations of the tiles\n"      <<
                    "h: print this information"                         << 
                    std::endl;
                break;
            }
            default: 
            {
                std::cout << action << " could not be interpreted, to see possible actions press 'h'" << std::endl;
                done = false; 
            }
        }
    }
}

void Scheduler::debugRun() 
{ 
    if (p_cpu == nullptr) 
    {
        std::cout << "Initialize the cpu, bozo";
        return;
    } 

    bool mode;
    int stop_at;

    std::cout << "Where are we stopping?" << std::endl;
    handleDebugStop(mode, stop_at);
    std::cout << std::endl;

    bool go_next = true;
    next_dot_time = steady_clock::now() + SYSTEM_CLOCKS_PER_DOT;

    //std::ofstream f("log");
    //f << cpu->toString() << std::endl;

    while (go_next && !glfwWindowShouldClose(screen.getWindow())) 
    {
        if (schedule.top().first <= time)
        {
            if 
            (
                (schedule.top().second == CPU_EXEC) && 
                (
                    (stop_at == -1)                             || 
                    (!mode && (stop_at == p_cpu->getPC()))        || 
                    (mode && (stop_at == memory[p_cpu->getPC()]))
                )
            )
            {
                std::cout << p_cpu->toString() << "\n" << p_cpu->getAsm() << "\n" << std::endl;
                handleDebugStop(mode, stop_at);
            }
            //if (!interrupt)
            //    log << cpu->toString() << std::endl;
            go_next = pop();
        }
        else
            tick();
    }
}


void Scheduler::stop() {}

void Scheduler::statInterruptCheck() 
{
    bool tmp = statInterruptLine;
    byte& lcd_stat = memory[Memory::LCD_STAT];
    u8 ppuMode = lcd_stat & 0b0011;
    statInterruptLine = ((lcd_stat & 0b01000000) && (lcd_stat & 0b0100))            || 
                        ((ppuMode != 3) && (lcd_stat & (0b00001000 << ppuMode)));

    if (tmp < statInterruptLine) 
        memory[Memory::INTERRUPT_FLAG] |= 0b0010;
}

void Scheduler::tick() 
{
    while(next_dot_time > std::chrono::steady_clock::now()) {}
    next_dot_time += SYSTEM_CLOCKS_PER_DOT;

    time += 4; 
}

