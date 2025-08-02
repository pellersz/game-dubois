#ifndef SCHEDULER_H 
#define SCHEDULER_H

#include "cpu.h"
#include "mem.h"

class Scheduler {
public:
    enum class Ticks {
        ONE = 1,
        TWO = 2,
        FOUR = 4
    };
    Scheduler(Cpu&, Memory&);
    void tick(Ticks);
private:
    unsigned long long time = 0;
    Cpu& cpu;
    Memory& memory;
};

#endif
