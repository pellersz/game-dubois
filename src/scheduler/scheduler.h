#ifndef SCHEDULER_H 
#define SCHEDULER_H

#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include <memory>
#include <queue>
#include "ppu.h"
#include "screen.h"
#include <unistd.h>
#include <chrono>

enum Process {
    CPU_EXEC,
    UPDATE_DIV,
    UPDATE_TIMA,
    VBLANK_START,
    VBLANK,
    LYC_LY_CMP,
    OAM_SCAN,
    DRAW_PIXELS,
    HBLANK,
    HANDLE_CONTROL
};

using namespace std::chrono;

typedef std::pair<unsigned long long, Process> ProcessStart;

class Scheduler {
public: 
    // should remove these
    static const unsigned int MASTER_CLOCK_FREQUENCY = 4194304;
    static const unsigned int SYSTEM_CLOCKS_PER_DOT;
    // unfortunaltely when I do that the display stops working properly for some reason

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

    constexpr static const auto SYSTEM_CLOCKS_PER_DOTT = round<system_clock::duration>(duration<double>(1./60));
    time_point<std::chrono::steady_clock> next_dot_time = steady_clock::now() + SYSTEM_CLOCKS_PER_DOTT;
    unsigned long long time = 0;

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
