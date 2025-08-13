#include "mem.h"
#include "types.h"
#include <cstring>
#include <iostream>

byte& Memory::operator[](unsigned short ind) { /*if(ind == 0xff44) { std::cout << "ly = " << std::hex << (int) memory[ind] << std::endl; if(memory[ind] == 0x84) exit(1); }*/ return memory[ind]; }

word Memory::operator()(unsigned short ind) { return ((word)memory[ind + 1] << 8) + (word)memory[ind]; }

void Memory::writeWord(unsigned short ind, word val)
{
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

void Memory::oamDma(byte val) { memcpy(memory + OAM, memory + 0x100 * val, 0x100); }

