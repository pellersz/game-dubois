#ifndef MBC1_H
#define MBC1_H 

#include "mbc.h"
#include "mem.h"
#include "types.h"

class Mbc1: public Mbc 
{
public:
    Mbc1(Cartridge&);

    virtual void writtenToRegister(Memory& memory, unsigned short addr, byte val) override;

private:
    const bool moreThan512Kb;
    const u8 bankNo;
    u8 initialMask = 0b11111111;
    u8 secondaryMask = 1 << 7;

    bool ramEnable = 0;
    u8 bankSelectorLow5 = 1;
    u8 bankOrRamSelector = 0;
    u8 lastBankSelector = 1;
    u8 lastRamSeclector = 0;

    void changedBankNo(Memory&);
};

#endif
