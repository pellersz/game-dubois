#ifndef MBC_H 
#define MBC_H

#include "mem.h"
#include <memory>

class Cartridge;

class Mbc 
{
public:
    unsigned short firstBankOffs  = 0;
    unsigned short secondBankOffs = 0x4000;
    unsigned short ramOffs;

    virtual void init(std::shared_ptr<Cartridge>);

    virtual void writeToRegister(unsigned short, byte) {}

protected:
    std::shared_ptr<Cartridge> p_cartridge = nullptr;
};

#endif 
