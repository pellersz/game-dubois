#include "scheduler.h"
#include "GLFW/glfw3.h"
#include "apu.h"
#include "channels.h"
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

Scheduler::Scheduler(Memory& memory, Controller& controller, Ppu& ppu, Screen& screen, Apu& apu) : 
    memory(memory), 
    controller(controller), 
    ppu(ppu),
    screen(screen),
    apu(apu),
    bootRomMapping(memory.buildIn(Memory::BOOT_ROM_MAPPING)),
    divider(memory.buildIn(Memory::DIVIDER_REGISTER)),
    timerControl(memory.buildIn(Memory::TIMER_CONTROL)),
    timerCounter(memory.buildIn(Memory::TIMER_COUNTER)),
    timerModulo(memory.buildIn(Memory::TIMER_MODULO)),
    interruptFlag(memory.buildIn(Memory::INTERRUPT_FLAG)),
    lcdStat(memory.buildIn(Memory::LCD_STAT)),
    lcdY(memory.buildIn(Memory::LCD_Y)),
    lcdCmp(memory.buildIn(Memory::LY_COMPARE)),
    nr10(memory.buildIn(Memory::NR10)),
    nr12(memory.buildIn(Memory::NR12)),
    nr22(memory.buildIn(Memory::NR22)),
    nr42(memory.buildIn(Memory::NR42)),
    nr43(memory.buildIn(Memory::NR43))
{ 
    last_boot_rom = memory.read(Memory::BOOT_ROM_MAPPING);

    push(0, CPU_EXEC);
    push(0, UPDATE_DIV);
    push(0, UPDATE_TIMA);
    push(0, VBLANK_START);
    push(0, LYC_LY_CMP);
    push(0, HANDLE_CONTROL);
    push(100, WAIT);
}

void Scheduler::init(std::shared_ptr<Cpu> cpu_ptr) { pCpu = cpu_ptr; }

void Scheduler::push(unsigned int duration, Process process) { schedule.push(ProcessStart(time + duration, process)); }

// This is acceptable because there are not that many events at a time

void Scheduler::remove(Process process) {
    std::priority_queue<
        ProcessStart,
        std::vector<ProcessStart>,
        ProcessGreater 
    > new_schedule;

    while (!schedule.empty()) 
    {
        if (schedule.top().second != process)
            new_schedule.push(schedule.top());
        schedule.pop();
    }

    schedule = new_schedule;
}

void Scheduler::replace(Process process, unsigned short duration) 
{
    remove(process);
    schedule.push(ProcessStart {time + duration, process});
}

bool Scheduler::pop() 
{
    bool go_next = true;

    switch (schedule.top().second) 
    {
        case CPU_EXEC:
        {
            // since only the cpu cares about controller input, it only should update before it
            // TODO: move this to the controller handling still
            controller.updatePressed();
            pCpu->executeNext();

            // std::cout << pCpu->toString() << " ";
            if ((last_boot_rom != bootRomMapping) && (bootRomMapping == 1))
            {
                last_boot_rom = bootRomMapping;
                go_next = false;
            }

            break; 
        }
        case UPDATE_DIV: 
        {
            push((unsigned short) 256, UPDATE_DIV);
            ++divider;

            break;
        } 
        case UPDATE_TIMA: 
        {
            short duration = TIMA_PERIODS[timerControl & 0b0011];
            push(duration, UPDATE_TIMA);
            if (timerControl & 0b0100)
            {
                ++timerCounter;
                if (!timerCounter) 
                {
                    timerCounter = timerModulo;
                    interruptFlag |= 0b0100;
                }
            }
            break;
        }
        case VBLANK_START: 
        { 
            interruptFlag |= 0b0001; 
            // TODO: these sould be in one vblank() call
            screen.updateFrame();
            ppu.resetWindowY();
            ++lcdY;

            push(456 * Ppu::TIME_UNIT, VBLANK);
 
            lcdStat = (lcdStat & 0b11111100) + 0b01;
            statInterruptCheck();

            break;
        }
        case VBLANK: 
        {
            if (++lcdY < 154) 
            {
                push(456 * Ppu::TIME_UNIT, VBLANK);
                break;
            }
            lcdY = 0;
            push(456 * Ppu::TIME_UNIT, OAM_SCAN);
            break;
        }
        case LYC_LY_CMP: 
        { 
            push(4, LYC_LY_CMP);
            if (lcdY == lcdCmp) 
            {
                if (last_ly != lcdY) 
                {
                    lcdStat |= 0b0100;
                    statInterruptCheck();
                    break;
                }
                last_ly = lcdY;
            }
            lcdStat &= 0b11111011;
            break;
        }
        case OAM_SCAN: 
        { 
            ppu.oamScan();

            lcdStat = (lcdStat & 0b11111100) | 0b10;
            statInterruptCheck();

            push(80 * Ppu::TIME_UNIT, DRAW_PIXELS);
            break;
        }
        case DRAW_PIXELS:
        {
            ppu.drawLine();
            push(172 * Ppu::TIME_UNIT, HBLANK);
            ++lcdY;

            lcdStat = (lcdStat & 0b11111100) | 0b11;
            statInterruptCheck();

            break;
        }
        case HBLANK:
        {
            ppu.hBlank();
            
            lcdStat = (lcdStat & 0b11111100) + 0b00;
            statInterruptCheck();

            if (lcdY < 143) 
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
        case CH1_SWEEP: 
        {
            if (apu.ch1Sweep())
            {
                u8 pace = (nr10 >> 4) & 0b0111;
                push(pace * MASTER_CLOCK_FREQUENCY / CH1_SWEEP_FREQUENCY, CH1_SWEEP);
            }
            break;
        }
        case CH1_ENVELOPE:
        { 
            u8 pace = nr12 & 0b0111;
            if (apu.envelope(Ch1) && pace)
                push(pace * MASTER_CLOCK_FREQUENCY / ENVELOPE_FREQUENCY, CH1_ENVELOPE);

            break;
        }
        case CH1_TIME: 
        {
            apu.incrementTimer(Ch1);
            push(MASTER_CLOCK_FREQUENCY / SOUND_TIMER_FREQUENCY, CH1_TIME);
            break;
        }
        case CH2_ENVELOPE:
        { 
            u8 pace = nr22 & 0b0111;
            if (apu.envelope(Ch2) && pace)
                push(pace * MASTER_CLOCK_FREQUENCY / ENVELOPE_FREQUENCY, CH2_ENVELOPE);

            break;
        }
        case CH2_TIME: 
        {
            apu.incrementTimer(Ch2);
            push(MASTER_CLOCK_FREQUENCY / SOUND_TIMER_FREQUENCY, CH2_TIME);
            break;
        }
        case CH3_TIME:
        {
            apu.incrementTimer(Ch3);
            push(MASTER_CLOCK_FREQUENCY / SOUND_TIMER_FREQUENCY, CH3_TIME);
            break;
        }
        case CH4_SHIFT:
        {
            apu.ch4Shift();

            float divider = nr43 & 0b0111;
            if (!divider)
                divider = 0.5;

            float shift = 1 << (nr43 >> 4);

            push(CH4_SHIFT_TIME / (divider * shift), CH4_SHIFT);
            break;
        }
        case CH4_ENVELOPE:
        {
            u8 pace = nr42 & 0b0111;
            if (apu.envelope(Ch4) && pace)
                push(pace * MASTER_CLOCK_FREQUENCY / ENVELOPE_FREQUENCY, CH4_ENVELOPE);

            break;
        }
        case CH4_TIME:
        {
            apu.incrementTimer(Ch4);
            push(MASTER_CLOCK_FREQUENCY / SOUND_TIMER_FREQUENCY, CH4_TIME);
            break;           
        }
        case SAMPLE: 
        { 
            apu.sample();
            push(88, SAMPLE);
            break; 
        }
        case WAIT:
        {
            // TODO: adjust because it seems a bit too fast
            if (next_dot_time > system_clock::now())
            {
                while (next_dot_time > system_clock::now());
                next_dot_time += WAITING_TIME;
            }
            else 
                next_dot_time = system_clock::now() + WAITING_TIME;

            push(TIME_BETWEEN_WAIT, WAIT);
            break;
        }
    };

    schedule.pop(); 
    return go_next;
}

void Scheduler::run() 
{ 
    if (pCpu == nullptr) 
    {
        std::cout << "Initialize the cpu, bozo";
        return;
    } 

    bool go_next = true;
    next_dot_time = system_clock::now() + WAITING_TIME;

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
                std::cout << std::hex << (int) memory.read(addr) << std::endl;
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
    if (pCpu == nullptr) 
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
    next_dot_time = system_clock::now() + WAITING_TIME;

    std::ofstream f("log");
    //f << pCpu->toString() << std::endl;

    while (go_next && !glfwWindowShouldClose(screen.getWindow())) 
    {
        if (schedule.top().first <= time)
        {
            if (schedule.top().second == CPU_EXEC)
            {
                if (pCpu->getPC() != 0x50)
                    f << pCpu->toString() << std::endl;
                if
                (
                    (stop_at == -1)                              || 
                    (!mode && (stop_at == pCpu->getPC()))        || 
                    (mode && (stop_at == memory.read(pCpu->getPC())))
                )
                {
                    std::cout << pCpu->toString() << "\n" << pCpu->getAsm() << "\n" << std::endl;
                    handleDebugStop(mode, stop_at);
                }
            }
            //if (!interrupt)
            go_next = pop();
        } else
            tick();
    }
}


void Scheduler::stop() {}

void Scheduler::statInterruptCheck() 
{
    bool tmp = statInterruptLine;
    u8 ppuMode = lcdStat & 0b0011;
    statInterruptLine = ((lcdStat & 0b01000000) && (lcdStat & 0b0100))            || 
                        ((ppuMode != 3) && (lcdStat & (0b00001000 << ppuMode)));

    if (tmp < statInterruptLine) 
        interruptFlag |= 0b0010;
}

void Scheduler::tick() 
{
    // TODO: do something with the time
    apu.tickPeriod(Ch1, 1);
    apu.tickPeriod(Ch2, 1);
    apu.tickPeriod(Ch3, 2);

    time += 4; 
}

