#ifndef MBC_H 
#define MBC_H

#include "mem.h"

class Cartridge;

class Mbc 
{
public:
    Mbc(Cartridge& cartridge): cartridge(cartridge) {}
    virtual void writtenToRegister(Memory&, unsigned short, byte) {}

protected:
    Cartridge& cartridge;
};

#endif 
