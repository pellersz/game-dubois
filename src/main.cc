#include "cpu.h"
#include "mem.h"
#include <iostream>

int main() {
    Memory memory;
    Cpu cpu(memory);
    memory[0] = 0;
    memory[1] = 1;
    memory[2] = 2;
    memory[3] = 3;
    unsigned short* a = (unsigned short *)&memory[1];
    *a += 0x100;
    // TODO: check what word + offs yields when negative
    std::cout << "I live " << (int)memory[0] << " " << (int)memory[1] << " " << (int)memory[2] << " " << (int)memory[3] << " " << " yes";
}
