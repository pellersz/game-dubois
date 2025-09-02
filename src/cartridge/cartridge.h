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
    byte *data;

    Cartridge(std::string);
    ~Cartridge();
   
    int getRomSize();
    int getRamSize();

    byte readBank(unsigned short);
    byte readBankN(unsigned short);
    byte readRam(unsigned short);
    void writeToRegister(unsigned short, byte);
    void writeToRam(unsigned short, byte);

private: 
    Mbc mbc;
    int romSize;
    int ramSize;

    long getFileSize(std::string); 
};

#endif
