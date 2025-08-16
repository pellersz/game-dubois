#include "ppu.h"
#include "mem.h"
#include "screen.h"
#include "types.h"
#include <iostream>
#include <iterator>

const byte Ppu::COLORS[] = {WHITE, LIGHT_GRAY, DARK_GRAY, BLACK};

Ppu::Ppu(Memory& memory, Screen& screen) : memory(memory), screen(screen) {}

void Ppu::oamScan() 
{
    byte lcdc = memory[Memory::LCD_CONTROL];
    bool is_16_bit = lcdc & 0b0100;
    
    numberOfObjects = 0;
    for(unsigned short obj_ind = 0xfe00; obj_ind < 0xfea0 && numberOfObjects < 10; obj_ind += 4) 
    {
        byte pos_y = memory[obj_ind] - 16;
        byte current_lcd_pos = memory[Memory::LCD_Y];
        if ((pos_y <= current_lcd_pos) && (pos_y + 8 + 8 * is_16_bit > current_lcd_pos)) {
            objects[numberOfObjects].posX         = memory[obj_ind + 1];
            objects[numberOfObjects].internalY    = pos_y - current_lcd_pos;
            objects[numberOfObjects].tileIndex    = memory[obj_ind + 2];
            objects[numberOfObjects++].attributes = memory[obj_ind + 3];
        }
    }
}

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

    if (data) std::cout << "f";
    for(u8 x = lcd_x + 7; x > lcd_x; --x) 
    {
        screen(lcd_y, x) = COLORS[color_indices[data & 0b0001 + ((data >> 14) & 0b0010)]];
        data >>= 1;
    }
    screen(lcd_y, lcd_x) = COLORS[color_indices[data & 0b0001 + ((data >> 14) & 0b0010)]];
}

void Ppu::drawBackground() 
{
    byte bg_pal_data = memory[Memory::BG_PAL_DATA];
    u8 color_indices[4] = 
    { 
        static_cast<u8>(bg_pal_data & 0b11), 
        static_cast<u8>((bg_pal_data >> 2) & 0b11),
        static_cast<u8>((bg_pal_data >> 4) & 0b11),
        static_cast<u8>((bg_pal_data >> 6)),
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
    int counter = 0;
    for (short lcd_x = 0 - x % 8; lcd_x < Screen::LCD_WIDTH; lcd_x += 8, ++tile_offs, --no_wrap) 
    { 
        if (!no_wrap)
            tile_offs -= TILE_WIDTH; 
    
        drawBackgroundTile(lcd_x, ly, memory[tile_offs], y_offs, color_indices, lcdc_bit_4); 
    }
}

void Ppu::drawWindow() 
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
    bool lcdc_bit_6 = lcdc & 0b01000000;
    u8 x  = memory[Memory::WINDOW_X];
    u8 y  = memory[Memory::WINDOW_Y];

    unsigned short tile_offs = (lcdc_bit_6 ? Memory::TILE_M1 : Memory::TILE_M0);

    u8 y_offs = y % TILE_WIDTH;

    if (win_y_counter > y && win_y_counter < Screen::LCD_HEIGHT) 
    {
        for (u8 lcd_x = x - x % 8; lcd_x < Screen::LCD_WIDTH; lcd_x += 8, ++tile_offs) 
            drawBackgroundTile(lcd_x, win_y_counter, memory[tile_offs], y_offs, color_indices, lcdc_bit_4);
    }
    ++win_y_counter;
}

void Ppu::drawObjectTile
(
    u8 lcd_x,
    u8 lcd_y,
    u8 tile_offs, 
    u8 y_offs, 
    bool attribute_bit_5,
    u8 color_indices[]
) 
{
    word data = memory(Memory::TILES + tile_offs * TILE_BYTE_SIZE + 2 * y_offs);
    
    offs orientation = attribute_bit_5 ? -1 : 1;
    for (u8 i = attribute_bit_5 ? 7 : 0; (i + 1) % 9; i += orientation)
    {
        u8 index = color_indices[data & 0b0001 + ((data >> 14) & 0b0010)];
        data >>= 1;
        if (index)
            screen(lcd_x++, lcd_y) = COLORS[index];
    }
} 
 
void Ppu::drawObjects() 
{
    byte ly = memory[Memory::LCD_Y];
    bool lcdc_bit_2 = memory[Memory::LCD_CONTROL] & 0b0100; 

    for (u8 i = 0; i < numberOfObjects; ++i)
    {
        ObjectLine object = objects[i];

        if ((!(object.attributes & 0b10000000)))
        {
            byte pixel_data = memory[Memory::TILES + object.tileIndex * TILE_BYTE_SIZE];
            bool attribute_bit_5 = object.attributes & 0b00100000;
            bool attribute_bit_4 = object.attributes & 0b00010000;
            u8 y_offs = lcdc_bit_2 ? 15 - object.internalY : object.internalY;

            byte obj_pal_data = attribute_bit_4 ? memory[Memory::OBJ_PAL_1_DATA] : memory[Memory::OBJ_PAL_0_DATA];
            u8 color_indices[4] = 
            { 
                0,
                static_cast<u8>((obj_pal_data >> 2) & 0b11),
                static_cast<u8>((obj_pal_data >> 4) & 0b11),
                static_cast<u8>((obj_pal_data >> 6) & 0b11),
            };

            drawObjectTile
            (
                object.posX, 
                ly,
                object.tileIndex,
                y_offs,
                attribute_bit_5,
                color_indices 
            );
        }
    }
}

void Ppu::hBlank() {}

void Ppu::vBlank() {}

void Ppu::printTiles() {
    const char t[] = {' ', '@', '@', '@'};
    for (int i = 0; i < 55; ++i) 
    {
        for (int j = 0; j < 8; ++j) 
        {
            for (int k = 0; k < 55; ++k) 
            {
                word data = memory(Memory::TILES + (i * 55 + k) * TILE_BYTE_SIZE + 2 * j);

                for(int x = 0; x < 8; x++) 
                {
                    std::cout << t[data & 0b0001 + ((data >> 14) & 0b0010)];
                    data >>= 1;
                }

                std::cout << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
