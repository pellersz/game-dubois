#ifndef MEM_H 
#define MEM_H

#include "types.h"

class Memory 
{
public:
    byte& operator[](unsigned short);
    word operator()(unsigned short);

    void writeWord(unsigned short, word);
    bool writeData(unsigned short, int, const byte*);
    void oamDma(byte);

    // starting addresses for memory sectors
    static const unsigned short ROMBANK0 = 0x0000;
    static const unsigned short ROMBANK1 = 0x4000;   
    static const unsigned short VRAM     = 0x8000;
    static const unsigned short TILES    = 0x8000;
    static const unsigned short TILE_M0  = 0x9800;
    static const unsigned short TILE_M1  = 0x9c00;
    static const unsigned short EXT_RAM  = 0xa000;
    static const unsigned short WRAM0    = 0xc000;
    static const unsigned short WRAM1    = 0xd000;
    static const unsigned short ECHO_RAM = 0xe000;  // This is not to be touched. Source: Nintendo
    static const unsigned short OAM      = 0xfe00;
    static const unsigned short NO_TOUCH = 0xfea0;  // As the names suggests, this is not to be touched. Source: Nintendo
    static const unsigned short IO_REGS  = 0xff00;
    static const unsigned short HRAM     = 0xff80;
    static const unsigned short IE_REG   = 0xffff;

    // IO registers
    static const unsigned short JOYPAD           = 0xff00;
    static const unsigned short SER_TRANS_DATA   = 0xff01;  
    static const unsigned short SER_TRANS_CTRL   = 0xff01;  
    static const unsigned short DIVIDER_REGISTER = 0xff04;  
    static const unsigned short TIMER_COUNTER    = 0xff05;  
    static const unsigned short TIMER_MODULO     = 0xff06;  
    static const unsigned short TIMER_CONTROL    = 0xff07;  // just one byte  
    static const unsigned short INTERRUPT_FLAG   = 0xff0f;

    static const unsigned short NR10             = 0xff10;  //NRxx: sound channel
    static const unsigned short NR11             = 0xff11;
    static const unsigned short NR12             = 0xff12;
    static const unsigned short NR13             = 0xff13;
    static const unsigned short NR14             = 0xff14;
    static const unsigned short NR21             = 0xff16;
    static const unsigned short NR22             = 0xff17;
    static const unsigned short NR23             = 0xff18;
    static const unsigned short NR24             = 0xff19;
    static const unsigned short NR30             = 0xff1a;
    static const unsigned short NR31             = 0xff1b;
    static const unsigned short NR32             = 0xff1c;
    static const unsigned short NR33             = 0xff1d;
    static const unsigned short NR34             = 0xff1e;
    static const unsigned short NR41             = 0xff20;
    static const unsigned short NR42             = 0xff21;
    static const unsigned short NR43             = 0xff22;
    static const unsigned short NR44             = 0xff23;
    static const unsigned short NR50             = 0xff24;
    static const unsigned short NR51             = 0xff25;
    static const unsigned short NR52             = 0xff26;

    static const unsigned short WAVE_PATTERN     = 0xff30;  
    static const unsigned short LCD_CONTROL      = 0xff40;
    static const unsigned short LCD_STAT         = 0xff41;
    static const unsigned short VIEW_Y           = 0xff42;
    static const unsigned short VIEW_X           = 0xff43;
    static const unsigned short LCD_Y            = 0xff44;
    static const unsigned short LY_COMPARE       = 0xff45;
    static const unsigned short OAM_DMA_ADDR     = 0xff46;
    static const unsigned short BG_PAL_DATA      = 0xff47; 
    static const unsigned short OBJ_PAL_0_DATA   = 0xff48;
    static const unsigned short OBJ_PAL_1_DATA   = 0xff49;
    static const unsigned short WINDOW_Y         = 0xff4a;
    static const unsigned short WINDOW_X         = 0xff4b;
    static const unsigned short CPU_MODE_SELECT  = 0xff4c;  // CGB only
    static const unsigned short PREP_SPD_SWITCH  = 0xff4d;  // just one byte + CGB only
    static const unsigned short VRAM_BANK_SELECT = 0xff4f;  // CGB
    static const unsigned short BOOT_ROM_MAPPING = 0xff50;
    static const unsigned short VRAM_DMA         = 0xff51;  //ends before 0xff56
    static const unsigned short BG_OBJ_PALLETTES = 0xff69;  //ends before 0xff6b
    static const unsigned short WRAM_BANK_SELECT = 0xff70; 

private: 
    byte memory[0x10000];
};

#endif
