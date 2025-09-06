#include "mbc1.h"
#include "cartridge.h"
#include "mbc.h"
#include "mem.h"
#include <iostream>
#include <memory>

// TODO: even more constants
void Mbc1::init(std::shared_ptr<Cartridge> p_cartridge) 
{
    Mbc::init(p_cartridge);
    moreThan512Kb = p_cartridge->getRomSize() > 512 * 1024;
    bankNo = p_cartridge->getRomSize() / 0x4000; 
    for (; !(secondaryMask & ((p_cartridge->getRomSize() / 0x4000) - 1)); secondaryMask >>= 1)
        initialMask &= ~secondaryMask;
    
    secondaryMask = ~secondaryMask;
}

void Mbc1::writeToRegister(unsigned short addr, byte val) 
{
    if      (addr < 0x2000)
        ramEnable = ((val & 0b1111) == 0xa);
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
        else 
            ramOffs = pCartridge->getRomSize() + bankOrRamSelector * 8 * 1024;

        if (bankMode)
        {
            bool done = false;
            for (u8 bank_or_ram_selector = bankOrRamSelector, mask = 0b0100; !done && bank_or_ram_selector; bank_or_ram_selector &= ~mask)
            {
                int new_first_bank_offs = 0x4000 * (bank_or_ram_selector << 5);
                if (new_first_bank_offs < pCartridge->getRomSize())
                {
                    firstBankOffs = new_first_bank_offs;
                    done = true;
                }
                mask >>= 1;
            }
            if (!done)
                firstBankOffs = 0;
        }
    }
    else 
    {
        bankMode = val & 0b0001;
        
        if (!bankMode) 
        {
            firstBankOffs = 0;
            return;
        }

        bool done = false;
        // TODO: dry
        for (u8 bank_or_ram_selector = bankOrRamSelector, mask = 0b0100; !done && bank_or_ram_selector; bank_or_ram_selector &= ~mask)
        {
            int new_first_bank_offs = 0x4000 * (bank_or_ram_selector << 5);
            if (new_first_bank_offs < pCartridge->getRomSize())
            {
                firstBankOffs = new_first_bank_offs;
                done = true;
            }
            mask >>= 1;
        }
        if (!done)
            firstBankOffs = 0;
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

