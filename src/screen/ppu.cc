#include "ppu.h"
#include "mem.h"
#include "screen.h"
#include "types.h"

const byte Ppu::COLORS[] = {WHITE, LIGHT_GRAY, DARK_GRAY, BLACK};

Ppu::Ppu(Memory& memory, Screen& screen) : memory(memory), screen(screen) {}

void Ppu::oamScan() 
{
    byte lcdc = memory[Memory::LCD_CONTROL];
    bool is_16_bit = lcdc & 0b0100;
    
    numberOfObjects = 0;
    for(unsigned short obj_ind = 0xfe00; obj_ind < 0xfea0 && numberOfObjects < 10; obj_ind += 4) 
    {
        byte pos_y = memory[obj_ind];
        byte currenct_lcd_pos = memory[Memory::LCD_Y];
        if ((pos_y <= currenct_lcd_pos) && (pos_y + 8 + 8 * is_16_bit >= currenct_lcd_pos)) {
            objects[numberOfObjects].posX   = memory[obj_ind + 1];
            objects[numberOfObjects].posX   = memory[obj_ind + 2];
            objects[numberOfObjects++].posX = memory[obj_ind + 3];
        }
    }
}

// TODO: test this
void Ppu::drawLine() 
{
    byte lcdc = memory[Memory::LCD_CONTROL];
    if ((lcdc & 0b0001) && (lcdc & 0b10000000))
    {
        drawBackground();
        if (lcdc & 0b00100000)
            drawWindow();
    }
    else 
        screen.fillWhite(memory[Memory::LCD_Y]);

    if (lcdc & 0b0010)
        drawObjects();
}

u8 Ppu::getIndexFromWord(word word) { return ((0x8000 & word) >> 14) + ((0x80 & word) >> 7); }

void Ppu::drawBackgroundTile
(
        u8 lcd_x,
        u8 lcd_y,
        u8 tile_offs, 
        u8 y_offs, 
        u8 color_indices[],
        bool lcdc_bit_4
) 
{
    word data = lcdc_bit_4 ? 
        memory(Memory::TILES +               tile_offs * TILE_BYTE_SIZE + 2 * y_offs) : 
        memory(Memory::TILES + 0x1000 + (int)tile_offs * TILE_BYTE_SIZE + 2 * y_offs) ;

    for(u8 x = lcd_x + 7; x >= lcd_x; --x)
    {
        screen(x, lcd_y) = COLORS[color_indices[data & 0b0001 + ((data >> 14) & 0b0010)]];
    }
}

void Ppu::drawBackground() 
{
    byte bg_pal_data = memory[Memory::BG_PAL_DATA];
    u8 color_indices[4] = 
    { 
        static_cast<u8>(bg_pal_data & 0b11), 
        static_cast<u8>((bg_pal_data >> 2) & 0b11),
        static_cast<u8>((bg_pal_data >> 4) & 0b11),
        static_cast<u8>((bg_pal_data >> 6) & 0b11),
    };

    byte lcdc = memory[Memory::LCD_CONTROL];
    bool lcdc_bit_4 = lcdc & 0b00010000;
    bool lcdc_bit_3 = lcdc & 0b00001000;
    u8 x  = memory[Memory::VIEW_X];
    u8 y  = memory[Memory::VIEW_Y];
    u8 ly = memory[Memory::LCD_Y]; 
   
    unsigned short tile_offs = 
        (lcdc_bit_3 ? Memory::TILE_M1 : Memory::TILE_M0) +
        ((ly + y) / TILE_WIDTH) * TILES_PER_ROW + 
        x / TILE_WIDTH;

    u8 y_offs = (ly + y) % TILE_WIDTH;

    u8 no_wrap = TILES_PER_ROW - x / TILE_WIDTH;
    // align for whole bytes

    for (u8 lcd_x = 0 - x % 8; lcd_x < Screen::LCD_WIDTH; lcd_x += 8, ++tile_offs, --no_wrap) 
    {
        if (!no_wrap)
            tile_offs -= TILE_WIDTH; 
    
        drawBackgroundTile(lcd_x, ly, memory[tile_offs], y_offs, color_indices, lcdc_bit_4);
    }
}

// TODO: abstract something so there is less repetition between this and the previous
void Ppu::drawWindow() {
    byte bg_pal_data = memory[Memory::BG_PAL_DATA];
    u8 color_indices[4] = 
    { 
        static_cast<u8>(bg_pal_data & 0b11), 
        static_cast<u8>((bg_pal_data >> 2) & 0b11),
        static_cast<u8>((bg_pal_data >> 4) & 0b11),
        static_cast<u8>((bg_pal_data >> 6) & 0b11),
    };

    byte lcdc = memory[Memory::LCD_CONTROL];
    bool lcdc_bit_4 = lcdc & 0b00010000;
    bool lcdc_bit_6 = lcdc & 0b01000000;
    u8 x  = memory[Memory::WINDOW_X];
    u8 y  = memory[Memory::WINDOW_Y];
    u8 ly = memory[Memory::LCD_Y];
   
    unsigned short tile_offs = (lcdc_bit_6 ? Memory::TILE_M1 : Memory::TILE_M0) + ((ly + y) / TILE_WIDTH) * TILES_PER_ROW + x / TILE_WIDTH;
    u8 y_offs = (ly + y) % TILE_WIDTH;

    u8 no_wrap = TILES_PER_ROW - x / TILE_WIDTH;
    // align for whole bytes

    for (u8 lcd_x = 0 - x % 8; lcd_x < Screen::LCD_WIDTH; lcd_x += 8, ++tile_offs, --no_wrap) 
    {
        if (!no_wrap)
            tile_offs -= TILE_WIDTH; 
    
        drawBackgroundTile(lcd_x, ly, memory[tile_offs], y_offs, color_indices, lcdc_bit_4);
    }

}

