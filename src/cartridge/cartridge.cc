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
        case 0: { mbc = new  Mbc(); usesRam = false; break; }
        case 1: { mbc = new Mbc1(); usesRam = false; break; }
        case 2: { mbc = new Mbc1(); usesRam =  true; break; }
        default: 
        { 
            std::cout << "cartridge type unimplemented or unrecognized" << std::endl;
            throw 3;
        }
    }

    switch (data[0x148]) 
    {
        case    0: { romSize =  32 * KB; break; }
        case    1: { romSize =  64 * KB; break; }
        case    2: { romSize = 128 * KB; break; }
        case    3: { romSize = 256 * KB; break; }
        case    4: { romSize = 512 * KB; break; }
        case    5: { romSize =   1 * MB; break; }
        case    6: { romSize =   2 * MB; break; }
        case    7: { romSize =   4 * MB; break; }
        case    8: { romSize =   8 * MB; break; }
        case 0x52: { romSize = (1024 + 128) * KB; break; }
        case 0x53: { romSize = (1024 + 256) * KB; break; }
        case 0x54: { romSize = (1024 + 512) * KB; break; }
        default: 
        {
            std::cout << "rom size unrecognized" << std::endl;
            throw 4;
        }
    }

    if (usesRam)
    {
        switch (data[0x149])
        {
            case 0: { ramSize =   0; break; }
            case 1: { ramSize =   0; break; }
            case 2: { ramSize =   8; break; }
            case 3: { ramSize =  32; break; }
            case 4: { ramSize = 128; break; }
            case 5: { ramSize =  64; break; }
            default: 
            {
                std::cout << "ram size unrecognized" << std::endl;
                throw 5;
            }
        }

        ramSize *= KB;
        if (ramSize)
            mbc->ramOffs = romSize;
    }

    mbc->init(std::shared_ptr<Cartridge>(this));
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

byte Cartridge::readBank(unsigned short offs) { return data[mbc->firstBankOffs + offs]; }

byte* Cartridge::getBankPointer(unsigned short offs) { return data + mbc->firstBankOffs + offs; }

byte Cartridge::readBankN(unsigned short offs) { return data[mbc->secondBankOffs + offs]; }

byte* Cartridge::getBankNPointer(unsigned short offs) { return data + mbc->secondBankOffs + offs; }

byte Cartridge::readRam(unsigned short offs) { return data[mbc->ramOffs + offs]; }

byte* Cartridge::getRamPointer(unsigned short offs) { return (mbc->ramOffs != -1) ? data + mbc->ramOffs + offs : NULL; }

void Cartridge::writeToRegister(unsigned short offs, byte val) { mbc->writeToRegister(offs, val); }

void Cartridge::writeToRam(unsigned short offs, byte val) { if (mbc->ramOffs != -1) data[mbc->ramOffs + offs] = val; }

