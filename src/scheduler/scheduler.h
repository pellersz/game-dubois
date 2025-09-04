#ifndef SCHEDULER_H 
#define SCHEDULER_H

#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include <queue>
#include "ppu.h"
#include "screen.h"
#include "apu.h"
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
    HANDLE_CONTROL,
    CH1_SWEEP,
    CH1_ENVELOPE,
    CH1_TIME,
    CH2_ENVELOPE,
    CH2_TIME,
    CH3_TIME,
    CH4_SHIFT,
    CH4_ENVELOPE,
    CH4_TIME,
    SAMPLE,
    WAIT
};

using namespace std::chrono;

typedef std::pair<unsigned long long, Process> ProcessStart;

class Scheduler 
{
public: 
    const static int MASTER_CLOCK_FREQUENCY = 4194304;
    const static int SYSTEM_CLOCK_FREQUENCY = MASTER_CLOCK_FREQUENCY / 4;
    const static int ENVELOPE_FREQUENCY = 64;
    const static int SOUND_TIMER_FREQUENCY = 256;
    const static int CH1_SWEEP_FREQUENCY = 128;
    const static int CH4_SHIFT_TIME = MASTER_CLOCK_FREQUENCY / 16;
    constexpr static const unsigned short TIMA_PERIODS[4] = { 1024, 16, 64, 256 };

    Scheduler(Memory&, Controller&, Ppu&, Screen&, Apu&);
    void init(std::shared_ptr<Cpu>);

    void push(unsigned int, Process);
    void remove(Process);
    void replace(Process, unsigned short);

    void writtenToMemory(unsigned short, byte);
    
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

    const static int TIME_BETWEEN_WAIT = 500;
    constexpr static const auto WAITING_TIME = TIME_BETWEEN_WAIT * round<system_clock::duration>(duration<double>(1.0 / 4194304));
    time_point<std::chrono::system_clock> next_dot_time = system_clock::now() + WAITING_TIME;
    unsigned long long time = 0;

    std::shared_ptr<Cpu> pCpu = nullptr;
    Memory& memory;
    Controller& controller;
    Ppu& ppu;
    Screen& screen;
    Apu& apu;

    byte& bootRomMapping;
    byte& divider;
    byte& timerControl;
    byte& timerCounter;
    byte& timerModulo;
    byte& interruptFlag;
    byte& lcdStat;
    byte& lcdY;
    byte& lcdCmp;

    byte& nr10;
    byte& nr12;
    byte& nr22;
    byte& nr42;
    byte& nr43;
    
    byte last_div;
    byte last_boot_rom;
    byte last_ly = 255;

    bool pop();
    void tick();
    void handleDebugStop(bool&, int&);
};

#endif
