#ifndef MBC_H 
#define MBC_H

#include "types.h"
#include <memory>

class Cartridge;
class Memory;

class Mbc 
{
public:
    int firstBankOffs  = 0;
    int secondBankOffs = 0x4000;
    int ramOffs = -1;
    bool ramEnabled = false;

    virtual void init(Cartridge* p_cartridge) { pCartridge = p_cartridge; };

    virtual void writeToRegister(unsigned short, byte) {}

protected:
    Cartridge *pCartridge;
};

#endif 
