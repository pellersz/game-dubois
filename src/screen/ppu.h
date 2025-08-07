#ifndef PPU_H
#define PPU_H

#include "screen.h"
#include "mem.h"
#include "types.h"

class Ppu {
public:
    Ppu(Memory&, Screen&);
    
private:
    static const byte WHITE      = 255;
    static const byte LIGHT_GRAY = 255 * (2 / 3);
    static const byte DARK_GRAY  = 255 * (1 / 3);
    static const byte BLACK      = 0;
    static const byte COLORS[4];

    static const unsigned short BG_PIXEL_WIDTH = 256;
    static const u8 TILE_WIDTH     = 8;
    static const u8 TILES_PER_ROW  = 32;
    static const u8 TILE_BYTE_SIZE = 16;
    static const u8 PIXELS;

    Memory& memory;
    Screen& screen;

    class ObjectLine {
    public:
        u8 posX;
        u8 tileIndex;
        byte attributes;

        ObjectLine() {};
        void draw();
    };
    
    u8 numberOfObjects = 0;
    ObjectLine objects[10];
    
    void oamScan();
    void drawLine();
    void hBlank();
    void vBlank();
    
    u8 getIndexFromWord(word);

    void drawBackgroundTile
(
        u8 lcd_x,
        u8 lcd_y,
        u8 tile_offs, 
        u8 y_offs, 
        u8 color_indices[],
        bool lcdc_bit_4
);

    void drawBackground();
    void drawWindow();
    void drawObjects();
};

#endif
