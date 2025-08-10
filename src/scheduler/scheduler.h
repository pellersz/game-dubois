#ifndef SCHEDULER_H 
#define SCHEDULER_H

#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include <ctime>
#include <memory>
#include <queue>
#include "ppu.h"
#include "screen.h"

enum Process {
    CPU_EXEC,
    UPDATE_DIV,
    UPDATE_TIMA,
    VBLANK,
    LYC_LY_CMP,
    OAM_SCAN,
    DRAW_PIXELS,
    HBLANK,
    HANDLE_CONTROL
};

typedef std::pair<unsigned long long, Process> ProcessStart;

class Scheduler {
public: 
    static const unsigned int MASTER_CLOCK_FREQUENCY = 4194304;
    static const unsigned int SYSTEM_CLOCKS_PER_DOT = CLOCKS_PER_SEC / MASTER_CLOCK_FREQUENCY;
    static const u8 CLOCKS_BETWEEN_DIV = 1;

    Scheduler(Memory&, Controller&, Ppu&, Screen&);
    void init(std::shared_ptr<Cpu>);

    void push(unsigned short, Process);
    bool pop();
    void run();
    void stop();

    void statInterruptCheck();

private: 
    bool statInterruptLine = false;

    class ProcessGreater
    {
    public:
        bool operator()(ProcessStart a, ProcessStart b) { return a.first > b.first; }
    };

    std::priority_queue<
        ProcessStart,
        std::vector<ProcessStart>,
        ProcessGreater 
    > schedule;

    unsigned long long time = 0;
    clock_t next_dot_time;
    std::shared_ptr<Cpu> cpu = nullptr;
    Memory& memory;
    Controller& controller;
    Ppu& ppu;
    Screen& screen;
    
    byte last_div;
    byte last_boot_rom;

    void tick();
};

#endif
