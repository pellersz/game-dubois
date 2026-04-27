#include "Mbc1.hpp"
#include "Cartridge.hpp"
#include "Mbc.hpp"
#include "Memory.hpp"

void Mbc1::init(Cartridge* pCartridge) 
{
    Mbc::init(pCartridge);

    moreThan512Kb = pCartridge->getRomSize() > 512 * KB;
    bankNo = pCartridge->getRomSize() / ROM_BANK_SIZE; 
    
    for (; !(secondaryMask & ((pCartridge->getRomSize() / ROM_BANK_SIZE) - 1)); secondaryMask >>= 1)
        initialMask &= ~secondaryMask;
    
    secondaryMask = ~secondaryMask;
}

void Mbc1::writeToRegister(unsigned short addr, byte val) 
{
    if      (addr < 0x2000)
        ramEnabled = ((val & 0b1111) == 0xa);
    else if (addr < 0x4000) 
    {
        bankSelectorLow5 = val & 0b00011111;
        
        if (!bankSelectorLow5)
            bankSelectorLow5 = 1;

        changedBankNo();
    }
    else if (addr < 0x6000)
    {
        bankOrRamSelector = val & 0b0011;
     
        if (moreThan512Kb)
            changedBankNo();
        
        if (bankMode)
            advancedBankModeAdjustOffset();
    }
    else 
    {
        bankMode = val & 0b0001;
        if (!bankMode) 
        {
            firstBankOffs = 0;
            
            ramOffs = pCartridge->getRomSize();
            return;
        }

        advancedBankModeAdjustOffset();
    }
}

void Mbc1::changedBankNo() 
{
    u8 newBankSelector = bankSelectorLow5;
    if (moreThan512Kb)
        newBankSelector += bankOrRamSelector << 5;

    newBankSelector &= initialMask;
    if (newBankSelector > bankNo)
        newBankSelector &= secondaryMask;

    secondBankOffs = newBankSelector * ROM_BANK_SIZE;
}

void Mbc1::advancedBankModeAdjustOffset() 
{
    bool done = false;

    for (u8 bankOrRamSelector = bankOrRamSelector, mask = 0b0100; !done && bankOrRamSelector; bankOrRamSelector &= ~mask)
    {
        int newFirstBankOffs = ROM_BANK_SIZE * (bankOrRamSelector << 5);
        
        if (newFirstBankOffs < pCartridge->getRomSize())
        {
            firstBankOffs = newFirstBankOffs;
            done = true;
        }
        mask >>= 1;
    }
    if (!done)
        firstBankOffs = 0;

    int newRamOffs = pCartridge->getRomSize() + bankOrRamSelector * 8 * KB;
    if (newRamOffs < pCartridge->getSize())
        ramOffs = newRamOffs;
}
