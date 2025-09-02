#ifndef MBC1_H
#define MBC1_H 

#include "mbc.h"
#include "mem.h"
#include "types.h"

class Mbc1: public Mbc 
{
public:
    virtual void init(std::shared_ptr<Cartridge>) override;
    virtual void writeToRegister(unsigned short, byte) override;

private:
    bool moreThan512Kb = false;
    bool bankMode = false;
    u8 bankNo;
    u8 initialMask = 0b11111111;
    u8 secondaryMask = 1 << 7;

    bool ramEnable = 0;
    u8 bankSelectorLow5 = 1;
    u8 bankOrRamSelector = 0;

    void changedBankNo();
};

#endif
