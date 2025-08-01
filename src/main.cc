#include "cpu.h"
#include "mem.h"
#include <iostream>

int main() {
    Memory memory;
    Cpu cpu(memory);
    memory[1] = 6;
    std::cout << "I live " << (int)memory[1] << " yes";
}
