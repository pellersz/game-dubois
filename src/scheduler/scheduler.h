#ifndef SCHEDULER_H 
#define SCHEDULER_H

#include "cpu.h"
#include "mem.h"
#include <memory>
#include <queue>
#include <utility>
#include <vector>

typedef std::pair<unsigned long long, unsigned short> ProcessStart;

enum Process {
    CPU_EXEC,
    UPDATE_DIV,
    UPDATE_TIMA
};

class Scheduler {
public: 
    static const u8 CLOCKS_BETWEEN_DIV = 1;

    Scheduler(Memory&);
    void init(std::shared_ptr<Cpu>);
    void push(u8, u8);
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

    void tick();
};

#endif
