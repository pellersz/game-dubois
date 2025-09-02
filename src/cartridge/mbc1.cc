#include "mbc1.h"
#include "cartridge.h"
#include "mbc.h"
#include "mem.h"
#include <cstring>

Mbc1::Mbc1(Cartridge& cartridge) : 
    Mbc(cartridge), 
    moreThan512Kb(cartridge.size > 512 * 1024),
    bankNo(cartridge.size / 0x4000 + (bool) (cartridge.size % 0x4000))
{
    for (; !(secondaryMask & cartridge.size); secondaryMask >>= 1)
        initialMask &= ~secondaryMask;
    secondaryMask = ~secondaryMask;
}

void Mbc1::writtenToRegister(Memory& memory, unsigned short addr, byte val) 
{
    if      (addr < 0x2000)
        ramEnable = ((val & 0b1111) == 0xa);
    else if (addr < 0x4000) 
    {
        u8 new_bank_selector_low_5 = val & 0b00011111;
        
        if (!new_bank_selector_low_5)
            new_bank_selector_low_5 = 1;

        changedBankNo(memory);
    }
    else if (addr < 0x6000)
    {
        bankOrRamSelector = val & 0b0011;
        if (moreThan512Kb)
            changedBankNo(memory);
        else 
        {

        }
    }
}

void Mbc1::changedBankNo(Memory& memory) 
{
    u8 new_bank_selector = new_bank_selector;
    if (moreThan512Kb)
        new_bank_selector += bankOrRamSelector << 5;

    new_bank_selector &= initialMask;
    if (new_bank_selector > bankNo)
        new_bank_selector &= secondaryMask;
    
    if (new_bank_selector != lastBankSelector)
        memory.writeData(0x4000, 0x4000, cartridge.rom + new_bank_selector * 0x4000);

    lastBankSelector = new_bank_selector;
}
