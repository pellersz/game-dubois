#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "mbc.h"
#include "mem.h"
#include "types.h"
#include <string>

class Cartridge 
{
public: 
    int size;
    byte *rom;
    Mbc mbc;

    Cartridge(std::string);
    ~Cartridge();
    void writtenToRegisters(Memory&, unsigned short, byte);

private: 
    long getFileSize(std::string); 
};

#endif
