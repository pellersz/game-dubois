#include "mem.h"

byte& Memory::operator[](unsigned short ind) { return memory[ind]; }

word Memory::operator()(unsigned short ind) { return ((word)memory[ind] << 8) + (word)memory[ind + 1]; }

void Memory::writeWord(unsigned short ind, word val) {
    memory[ind] = val >> 8;       
    memory[ind + 1] = val;
}

