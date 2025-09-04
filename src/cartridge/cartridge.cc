#include "cartridge.h"
#include "mbc.h"
#include "mbc1.h"
#include "mem.h"
#include "types.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

Cartridge::Cartridge(std::string filename)
{
    size = getFileSize(filename);
    if (size == -1) 
    {
        std::cout << "\"" << filename << "\": could not get file size" << std::endl;
        throw 1;
    }
    data = new byte[size];
    
    FILE* file = fopen(filename.c_str(), "r");
    if (file == NULL)
    {
        std::cout << "\"" << filename << "\": could not open file for reading" << std::endl;
        throw 2;
    }

    fread(data, 1, size, file); 

    bool usesRam;
    switch (data[0x147])
    {
        case 0: { mbc =  Mbc(); usesRam = false; break; }
        case 1: { mbc = Mbc1(); usesRam = false; break; }
        case 2: { mbc = Mbc1(); usesRam =  true; break; }
    }

    switch (data[0x148]) 
    {
        case    0: { romSize =       32; break; }
        case    1: { romSize =       64; break; }
        case    2: { romSize =      128; break; }
        case    3: { romSize =      256; break; }
        case    4: { romSize =      512; break; }
        case    5: { romSize =     1024; break; }
        case    6: { romSize = 2 * 1024; break; }
        case    7: { romSize = 4 * 1024; break; }
        case    8: { romSize = 8 * 1024; break; }
        case 0x52: { romSize = (1024 + 128); break; }
        case 0x53: { romSize = (1024 + 256); break; }
        case 0x54: { romSize = (1024 + 512); break; }
    }
    romSize *= 1024;

    switch (data[0x149])
    {
        case 0: { ramSize =   0; break; }
        case 1: { ramSize =   0; break; }
        case 2: { ramSize =   8; break; }
        case 3: { ramSize =  32; break; }
        case 4: { ramSize = 128; break; }
        case 5: { ramSize =  64; break; }
    }
    ramSize *= 1024;

    mbc.init(std::make_shared<Cartridge>(*this));
}

Cartridge::~Cartridge() { delete[] data; }

long Cartridge::getFileSize(std::string filename) 
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int Cartridge::getRomSize() { return romSize; }

int Cartridge::getRamSize() { return ramSize; }

byte Cartridge::readBank(unsigned short addr) { return data[mbc.firstBankOffs + addr]; }

byte* Cartridge::getBankPointer(unsigned short addr) { return data + mbc.firstBankOffs + addr; }

byte Cartridge::readBankN(unsigned short addr) { return data[mbc.secondBankOffs + addr]; }

byte* Cartridge::getBankNPointer(unsigned short addr) { return data + mbc.secondBankOffs + addr; }

byte Cartridge::readRam(unsigned short addr) { return data[mbc.ramOffs + addr]; }

byte* Cartridge::getRamPointer(unsigned short addr) { return data + mbc.ramOffs + addr; }

void Cartridge::writeToRegister(unsigned short addr, byte val) { mbc.writeToRegister(addr, val); }

void Cartridge::writeToRam(unsigned short addr, byte val) { data[mbc.ramOffs + addr] = val; }

