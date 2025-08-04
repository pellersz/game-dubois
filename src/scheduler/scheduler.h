#ifndef SCHEDULER_H 
#define SCHEDULER_H

#include "controller.h"
#include "cpu.h"
#include "mem.h"
#include <memory>
#include <queue>
#include <utility>
#include <vector>

enum Process {
    CPU_EXEC,
    UPDATE_DIV,
    UPDATE_TIMA
};

typedef std::pair<unsigned long long, Process> ProcessStart;

class Scheduler {
public: 
    static const u8 CLOCKS_BETWEEN_DIV = 1;

    Scheduler(Memory&, Controller&);
    void init(std::shared_ptr<Cpu>);
    void push(u8, Process);
    void pop();
    void run();
    void stop();

private: 
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
    std::shared_ptr<Cpu> cpu;
    Memory& memory;
    Controller& controller;

    void tick();
};

#endif
