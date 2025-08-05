#include "mem.h"
#include <cstring>
#include <iostream>

byte& Memory::operator[](unsigned short ind) { return memory[ind]; }

word Memory::operator()(unsigned short ind) { return ((word)memory[ind] << 8) + (word)memory[ind + 1]; }

void Memory::writeWord(unsigned short ind, word val) {
    memory[ind] = val >> 8;       
    memory[ind + 1] = val;
}

bool Memory::writeData(unsigned short offs, unsigned short count, const byte* buf) 
{
    if ((int)(offs + count) > 0xffff)
    {
        std::cout << "Too much memory to be copied";
        return false;
    }
    memcpy(memory + offs, buf, count);
    return true;
}

