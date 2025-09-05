#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "mbc.h"
#include "types.h"
#include <string>

class Memory;

class Cartridge 
{
public:
    // TODO: at this point have this as a private
    int size;
    byte *data;

    Cartridge(std::string);
    ~Cartridge();
   
    int getRomSize();
    int getRamSize();

    byte readBank(unsigned short);
    byte *getBankPointer(unsigned short);
    byte readBankN(unsigned short);
    byte *getBankNPointer(unsigned short);
    byte readRam(unsigned short);
    byte *getRamPointer(unsigned short);
    void writeToRegister(unsigned short, byte);
    void writeToRam(unsigned short, byte);

private: 
    int romSize;
    int ramSize = 0;
    Mbc *mbc;

    long getFileSize(std::string); 
};

#endif
