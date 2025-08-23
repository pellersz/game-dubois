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

enum Process 
{
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

class Scheduler 
{
public: 
    constexpr static const unsigned short TIMA_PERIODS[4] = { 1024, 16, 64, 256 };

    Scheduler(Memory&, Controller&, Ppu&, Screen&);
    void init(std::shared_ptr<Cpu>);

    void push(unsigned short, Process);
    void replace(Process, unsigned short);
    
    void run();
    void debugRun();
    void stop();

    void statInterruptCheck();

private: 

    bool statInterruptLine = false;

    class ProcessGreater
    {
    public:
        bool operator()(ProcessStart a, ProcessStart b) { return (a.first > b.first) || ((a.first == b.first) && (a.second > b.second)); }
    };

    std::priority_queue
    <
        ProcessStart,
        std::vector<ProcessStart>,
        ProcessGreater 
    > schedule;

    constexpr static const auto SYSTEM_CLOCKS_PER_DOT = round<system_clock::duration>(duration<double>(1.0 / 1048576));
    time_point<std::chrono::steady_clock> next_dot_time = steady_clock::now() + SYSTEM_CLOCKS_PER_DOT;
    unsigned long long time = 0;

    std::shared_ptr<Cpu> p_cpu = nullptr;
    Memory& memory;
    Controller& controller;
    Ppu& ppu;
    Screen& screen;
    
    byte last_div;
    byte last_boot_rom;
    byte last_ly = 255;

    bool pop();
    void tick();
    void handleDebugStop(bool&, int&);
};

#endif
