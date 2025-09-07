#include "cartridge.h"
#include "mbc.h"
#include "mbc1.h"
#include "mem.h"
#include "types.h"
#include <cstdio>
#include <cstring>
#include <fstream>
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
        
    std::ifstream f(filename);
    if (f.fail())
    {
        std::cout << "\"" << filename << "\": could not open file for reading" << std::endl;
        throw 2;
    }
    data.resize(size);
    f.read((char *)data.data(), size);

    name = filename;
    if (name.substr(name.length() - 3, 3) == ".gb")
        name = name.substr(0, name.length() - 3);

    bool usesRam;
    switch (data[0x147])
    {
        case 0: { pMbc = std::make_unique<Mbc >(); usesRam = false; batteryBacked = false; break; }
        case 1: { pMbc = std::make_unique<Mbc1>(); usesRam = false; batteryBacked = false; break; }
        case 2: { pMbc = std::make_unique<Mbc1>(); usesRam =  true; batteryBacked = false; break; }
        case 3: { pMbc = std::make_unique<Mbc1>(); usesRam =  true; batteryBacked =  true; break; }
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

    if (size != romSize)
        std::cout << "Warning: the rom's size does not match the rom size indicated by the header" << std::endl;

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
        {
            pMbc->ramOffs = size;
            size += ramSize;
            data.resize(size);
        }
    }

    pMbc->init(this);
}

Cartridge::~Cartridge() 
{
    if (batteryBacked)
    {
        std::string save_name = name + ".sav";
        std::ofstream f(save_name);
        f.write((char *)data.data() + romSize, ramSize);
    }
}

long Cartridge::getFileSize(std::string filename) 
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int Cartridge::getSize() { return size; }

int Cartridge::getRomSize() { return romSize; }

int Cartridge::getRamSize() { return ramSize; }

byte Cartridge::readBank(unsigned short offs) { return data[pMbc->firstBankOffs + offs]; }

byte* Cartridge::getBankPointer(unsigned short offs) { return data.data() + pMbc->firstBankOffs + offs; }

byte Cartridge::readBankN(unsigned short offs) { return data[pMbc->secondBankOffs + offs]; }

byte* Cartridge::getBankNPointer(unsigned short offs) { return data.data() + pMbc->secondBankOffs + offs; }

byte Cartridge::readRam(unsigned short offs) 
{ 
    if (pMbc->ramEnabled) 
        return data[pMbc->ramOffs + offs]; 
    return 0xff;
}

byte* Cartridge::getRamPointer(unsigned short offs) { return (pMbc->ramOffs != -1) ? data.data() + pMbc->ramOffs + offs : NULL; }

void Cartridge::writeToRegister(unsigned short offs, byte val) { pMbc->writeToRegister(offs, val); }

void Cartridge::writeToRam(unsigned short offs, byte val) 
{ 
    if (pMbc->ramEnabled) 
        data[pMbc->ramOffs + offs] = val; 
}

