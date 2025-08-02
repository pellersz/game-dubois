#ifndef MEM_H 
#define MEM_H

#include "types.h"

class Memory {
public:
    byte& operator[](unsigned short);
    word operator()(unsigned short);
    void writeWord(unsigned short, word);

private:
    // starting addresses for memory sectors
    static const unsigned short ROMBANK0 = 0x0000;
    static const unsigned short ROMBANK1 = 0x4000;  // RombanK0 cannot be used 
    static const unsigned short VRAM     = 0x8000;
    static const unsigned short EXT_RAM  = 0xa000;
    static const unsigned short WRAM0    = 0xc000;
    static const unsigned short WRAM1    = 0xd000;
    static const unsigned short ECHO_RAM = 0xe000;  // This is not to be touched. Source: Nintendo
    static const unsigned short OAM      = 0xfe00;
    static const unsigned short NO_TOUCH = 0xfea0;  // As the names suggests, this is not to be touched. Source: Nintendo
    static const unsigned short IO_REGS  = 0xff00;
    static const unsigned short HRAM     = 0xff80;
    static const unsigned short IE_REG   = 0xffff;

    // IO ranges
    static const unsigned short JOYPAD           = 0xff00;
    static const unsigned short SERIAL_TRANSFER  = 0xff01;  // ends before 0xff03
    static const unsigned short DIVIDER_REGISTER = 0xff04;  
    static const unsigned short TIMER_COUNTER    = 0xff05;  
    static const unsigned short TIMER_MODULO     = 0xff06;  
    static const unsigned short TIMER_CONTROL    = 0xff07;  // just one byte  
    static const unsigned short INTERRUPT_FLAG   = 0xff0f;
    static const unsigned short AUDIO            = 0xff10;  // ends before 0xff27
    static const unsigned short WAVE_PATTERN     = 0xff30;  
    static const unsigned short MISC             = 0xff40;  // ends before 0xff4c
    static const unsigned short VRAM_BANK_SELECT = 0xff4f;
    static const unsigned short BOOT_ROM_MAPPING = 0xff50;
    static const unsigned short VRAM_DMA         = 0xff51;  //ends before 0xff56
    static const unsigned short BG_OBJ_PALLETTES = 0xff69;  //ends before 0xff6b
    static const unsigned short WRAM_BANK_SELECT = 0xff70; 

    byte memory[0x10000];
};

#endif
