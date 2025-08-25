#include "ppu.h"
#include "mem.h"
#include "screen.h"
#include "types.h"
#include <iostream>

Ppu::Ppu(Memory& memory, Screen& screen) : memory(memory), screen(screen) {}

void Ppu::oamScan() 
{
    byte lcdc = memory[Memory::LCD_CONTROL];
    bool is_16_bit = lcdc & 0b0100;
 
    numberOfObjects = 0;
    for (unsigned short obj_ind = 0xfe00; obj_ind < 0xfea0 && numberOfObjects < 10; obj_ind += 4) 
    {
        short pos_y = memory[obj_ind] - 16;
        byte current_lcd_pos = memory[Memory::LCD_Y];
        if ((pos_y <= current_lcd_pos) && (pos_y + 8 + 8 * is_16_bit > current_lcd_pos)) 
        {
            objects[numberOfObjects].posX         = memory[obj_ind + 1];
            objects[numberOfObjects].internalY    = current_lcd_pos - pos_y;
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
    bool bg_area
) 
{
    word data = bg_area ? 
        memory(Memory::TILES +                 tile_offs * TILE_BYTE_SIZE + 2 * y_offs) : 
        memory(Memory::TILES + 0x1000 + (offs) tile_offs * TILE_BYTE_SIZE + 2 * y_offs) ;

    for(u8 x = lcd_x + 7; x > lcd_x; --x) 
    {
        screen(lcd_y, x) = COLORS[color_indices[(data & 0b0001) + ((data >> 7) & 0b0010)]];
        data >>= 1;
    } 
    screen(lcd_y, lcd_x) = COLORS[color_indices[(data & 0b0001) + ((data >> 7) & 0b0010)]];
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
    bool bg_area = lcdc & 0b00010000;
    bool bg_tile_area = lcdc & 0b00001000;
    u8 x  = memory[Memory::VIEW_X];
    u8 y  = memory[Memory::VIEW_Y];
    u8 ly = memory[Memory::LCD_Y]; 
   
    unsigned short tile_offs = 
        (bg_tile_area ? Memory::TILE_M1 : Memory::TILE_M0) +
        ((u8)(ly + y) / TILE_WIDTH) * TILES_PER_ROW + 
        x / TILE_WIDTH;

    u8 y_offs = (ly + y) % TILE_WIDTH;

    u8 no_wrap = TILES_PER_ROW - x / TILE_WIDTH;
    
     // align for whole bytes
    for (short lcd_x = 0 - x % TILE_WIDTH; lcd_x < Screen::LCD_WIDTH; lcd_x += TILE_WIDTH, ++tile_offs, --no_wrap) 
    { 
        if (!no_wrap)
            tile_offs -= TILES_PER_ROW; 
    
        drawBackgroundTile(lcd_x, ly, memory[tile_offs], y_offs, color_indices, bg_area); 
    }
}

void Ppu::drawWindow() 
{
    int win_x = memory[Memory::WINDOW_X] - 7;
    u8 win_y  = memory[Memory::WINDOW_Y];
    u8 lcd_y = memory[Memory::LCD_Y];

    if (win_x < 160 && lcd_y >= win_y) 
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
        bool window_area = lcdc & 0b00010000;
        bool window_tile_area = lcdc & 0b01000000;

        unsigned short tile_offs = 
            (window_tile_area ? Memory::TILE_M1 : Memory::TILE_M0) +
            (winYCounter / TILE_WIDTH) * TILES_PER_ROW; 

        u8 y_offs = winYCounter % TILE_WIDTH;

        for (u8 lcd_x = win_x; lcd_x < Screen::LCD_WIDTH; lcd_x += 8, ++tile_offs) 
            drawBackgroundTile
            (
                lcd_x,
                lcd_y,
                memory[tile_offs],
                y_offs,
                color_indices,
                window_area
            );
        ++winYCounter;
    }
}

void Ppu::drawObjectTile
(
    u8 lcd_x,
    u8 lcd_y,
    u8 tile_offs, 
    u8 y_offs, 
    bool x_flip,
    bool priority,
    u8 color_indices[]
) 
{
    byte not_prio_color;
    if (priority) 
        not_prio_color = COLORS[memory[Memory::BG_PAL_DATA] & 0b11];

    word data = memory(Memory::TILES + tile_offs * TILE_BYTE_SIZE + 2 * y_offs);
   
    offs orientation = x_flip ? 1 : -1;
    // TODO: while this is very cool, it is also slow, just make two cases
    for (int i = x_flip ? 0 : 7; (i + 1) % 9; i += orientation)
    {
        u8 index_index = (data & 0b0001) + ((data >> 7) & 0b0010);
        data >>= 1;
        if (index_index && (!priority || (screen(lcd_y, lcd_x + i) == not_prio_color)))
            screen(lcd_y, lcd_x + i) = COLORS[color_indices[index_index]];
    }
} 
 
void Ppu::drawObjects() 
{
    byte lcd_y = memory[Memory::LCD_Y];
    bool is_16_bit = memory[Memory::LCD_CONTROL] & 0b0100; 

    for (int i = numberOfObjects - 1; i >= 0; --i)
    { 
        ObjectLine object = objects[i];

        u8 lcd_x = object.posX - 8;
        u8 tile_offs = is_16_bit ? object.tileIndex & 0xfe : object.tileIndex;        
 
        bool y_flip = object.attributes & 0b01000000;
        u8 y_offs = y_flip 
            ? (7 + is_16_bit * 8 - object.internalY) 
            : object.internalY;

        bool x_flip = object.attributes & 0b00100000;
        bool priority = object.attributes & 0b10000000;

        bool palette = object.attributes & 0b00010000;
        byte obj_pal_data = memory[palette ? Memory::OBJ_PAL_1_DATA : Memory::OBJ_PAL_0_DATA];
        u8 color_indices[4] = 
        { 
            0,
            static_cast<u8>((obj_pal_data >> 2) & 0b11),
            static_cast<u8>((obj_pal_data >> 4) & 0b11),
            static_cast<u8>((obj_pal_data >> 6) & 0b11),
        };

        drawObjectTile
        (
            object.posX - 8, 
            lcd_y,
            tile_offs,
            y_offs,
            x_flip,
            priority,
            color_indices 
        );
    }
}

void Ppu::hBlank() {}

void Ppu::vBlank() {}

void Ppu::printTiles() 
{
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
                    std::cout << t[(data & 0b0001) + ((data >> 7) & 0b0010)];
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

void Ppu::printUsedTiles() 
{
    const char t[] = {' ', '@', '@', '@'};
    int dummy;
    for (int v; v < 4; ++v) 
    {
        for (int i = 0; i < 32; ++i) 
        {
            for (int j = 0; j < 8; ++j) 
            {
                for (int k = 0; k < 33; ++k) 
                {
                    u8 tile_offs = memory
                    [
                        (v & 1 ? Memory::TILE_M1 : Memory::TILE_M0) +
                        i * TILES_PER_ROW + k
                    ];
                    word data = v & 2 ? 
                        memory(Memory::TILES +               tile_offs * TILE_BYTE_SIZE + 2 * j) : 
                        memory(Memory::TILES + 0x1000 + (offs) tile_offs * TILE_BYTE_SIZE + 2 * j) ;

                    for(int x = 0; x < 8; x++) 
                    {
                        std::cout << t[(data & 0b0001) + ((data >> 7) & 0b0010)];
                        data >>= 1;
                    }
                }
                std::cout << std::endl;
            }
        }
        std::cin >> dummy;
    }
    std::cout << std::endl;
}

void Ppu::resetWindowY() { winYCounter = 0; }
