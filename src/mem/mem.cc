#include "mem.h"
#include "types.h"
#include <cstring>
#include <iostream>
#include "cartridge.h"

byte Memory::read(unsigned short addr)
{
    switch (addr >> 12)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            return p_cartridge->readBank(addr);
        case 4:
        case 5:
        case 6:
        case 7:
            return p_cartridge->readBankN(addr - 0x4000);
        case 8:
        case 9:
            return videoRam[addr - 0x8000];
        case 10:
        case 11:
            return p_cartridge->readRam(addr - 0xa000);
        case 12:
        case 13:
            return workRam[addr - 0xc000];
        case 14:
            return 0xff;
        case 15:
        {
            if (addr < 0xfe00)
                return 0xff;
            else if (addr < 0xfea0)
                return oam[addr - 0xfe00];
            else if (addr < 0xff00)
                return 0xff;
            else 
                return last0x100[addr - 0xff00];
        }
        default:
            return 0xff;
    }
}

void Memory::write(unsigned short addr, byte val)
{
    switch (addr >> 12)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        {
            p_cartridge->writeToRegister(addr, val);
            return;
        }
        case 8:
        case 9:
        {
            videoRam[addr - 0x8000] = val;
            return;
        }
        case 10:
        case 11:
        {
            p_cartridge->writeToRam(addr - 0xa000, val);
            return;
        }
        case 12:
        case 13:
        {
            workRam[addr - 0xc000] = val;
            return;
        }
        case 14:
            return;
        case 15:
        {
            if (addr < 0xfe00)
                return;
            else if (addr < 0xfea0)
                oam[addr - 0xfe00] = val;
            else if (addr < 0xff00)
                return;
            else 
                last0x100[addr - 0xff00] = val;
        }
        default:
            return;
    }

}

word Memory::operator()(unsigned short ind) { return (read(ind + 1) << 8) + read(ind); }

void Memory::writeWord(unsigned short ind, word val)
{
    memory[ind] = val >> 8;       
    memory[ind + 1] = val;
}

bool Memory::writeData(unsigned short offs, int count, const byte* buf) 
{
    if ((int)(offs + count) > 0x10000)
    {
        std::cout << "Too much memory to be copied";
        return false;
    }
    memcpy(memory + offs, buf, count);
    return true;
}

void Memory::oamDma(byte val) { memcpy(memory + OAM, memory + 0x100 * val, 0x100); }

