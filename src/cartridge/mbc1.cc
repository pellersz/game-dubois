#include "mbc1.h"
#include "cartridge.h"
#include "mbc.h"
#include "mem.h"
#include <memory>

void Mbc1::init(std::shared_ptr<Cartridge> p_cartridge) 
{
    Mbc::init(p_cartridge);
    moreThan512Kb = p_cartridge->getRomSize() > 512 * 1024;
    bankNo = p_cartridge->size / 0x4000 + (bool) (p_cartridge->size % 0x4000); 
    for (; !(secondaryMask & p_cartridge->size); secondaryMask >>= 1)
        initialMask &= ~secondaryMask;
    secondaryMask = ~secondaryMask;
}

void Mbc1::writeToRegister(unsigned short addr, byte val) 
{
    if      (addr < 0x2000)
        ramEnable = ((val & 0b1111) == 0xa);
    else if (addr < 0x4000) 
    {
        u8 bankSelectorLow5 = val & 0b00011111;
        
        if (!bankSelectorLow5)
            bankSelectorLow5 = 1;

        changedBankNo();
    }
    else if (addr < 0x6000)
    {
        bankOrRamSelector = val & 0b0011;
        if (moreThan512Kb)
            changedBankNo();
        else 
            ramOffs = p_cartridge->getRomSize() + bankOrRamSelector * 8 * 1024;
    }
    else 
    {
        bankMode = val & 0b0001;
        firstBankOffs = bankMode ? 0x4000 * (bankOrRamSelector << 7) : 0;
    }
}

void Mbc1::changedBankNo() 
{
    u8 new_bank_selector = bankSelectorLow5;
    if (moreThan512Kb)
        new_bank_selector += bankOrRamSelector << 5;

    new_bank_selector &= initialMask;
    if (new_bank_selector > bankNo)
        new_bank_selector &= secondaryMask;

    secondBankOffs = new_bank_selector * 0x4000;
}

