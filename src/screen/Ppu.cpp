#include "Ppu.hpp"
#include "Memory.hpp"
#include "Screen.hpp"
#include "types.h"
#include <iostream>

Ppu::Ppu(Memory& memory, Screen& screen) : 
    memory(memory), 
    screen(screen),
    lcdc(memory.buildIn(Memory::LCD_CONTROL)),
    lcdY(memory.buildIn(Memory::LCD_Y)),
    bgPalData(memory.buildIn(Memory::BG_PAL_DATA)),
    viewX(memory.buildIn(Memory::VIEW_X)),
    viewY(memory.buildIn(Memory::VIEW_Y)),
    winX(memory.buildIn(Memory::WINDOW_X)),
    winY(memory.buildIn(Memory::WINDOW_Y)),
    oam(memory.getDataPointerToAddress(Memory::OAM))
{}

void Ppu::oamScan() 
{
    bool is16Bit = lcdc & 0b0100;
 
    numberOfObjects = 0;
    for (unsigned short objInd = 0; objInd < 0xa0 && numberOfObjects < 10; objInd += 4) 
    {
        short posY = oam[objInd] - 16;
        if ((posY <= lcdY) && (posY + 8 + 8 * is16Bit > lcdY)) 
        {
            objects[numberOfObjects].posX         = oam[objInd + 1];
            objects[numberOfObjects].internalY    = lcdY - posY;
            objects[numberOfObjects].tileIndex    = oam[objInd + 2];
            objects[numberOfObjects++].attributes = oam[objInd + 3];
        }
    }

}

void Ppu::drawLine() 
{
    if ((lcdc & 0b0001) && (lcdc & 0b10000000))
    {
        drawBackground();
        if (lcdc & 0b00100000)
            drawWindow();
    }
    else 
        screen.fillWhite(lcdY);

    if (lcdc & 0b0010)
        drawObjects();
}

u8 Ppu::getIndexFromWord(word word) { return ((0x8000 & word) >> 14) + ((0x80 & word) >> 7); }

void Ppu::drawBackgroundTile
(
    u8 lcdX,
    u8 lcdY,
    u8 tileOffs, 
    u8 yOffs, 
    u8 *colorIndices,
    bool bgArea
) 
{
    word data = bgArea ? 
        memory(Memory::TILES +                 tileOffs * TILE_BYTE_SIZE + 2 * yOffs) : 
        memory(Memory::TILES + 0x1000 + (offs) tileOffs * TILE_BYTE_SIZE + 2 * yOffs) ;

    for(short x = lcdX + 7; x > lcdX; --x) 
    {
        screen(lcdY, x) = COLORS[colorIndices[(data & 0b0001) + ((data >> 7) & 0b0010)]];
        data >>= 1;
    } 
    screen(lcdY, lcdX) = COLORS[colorIndices[(data & 0b0001) + ((data >> 7) & 0b0010)]];
}

void Ppu::drawBackground() 
{
    u8 colorIndices[4] = 
    { 
        static_cast<u8>(bgPalData & 0b11), 
        static_cast<u8>((bgPalData >> 2) & 0b11),
        static_cast<u8>((bgPalData >> 4) & 0b11),
        static_cast<u8>((bgPalData >> 6)),
    };

    bool bgArea = lcdc & 0b00010000;
    bool bgTileArea = lcdc & 0b00001000;
   
    unsigned short tileOffs = 
        (bgTileArea ? Memory::TILE_M1 : Memory::TILE_M0) +
        ((u8)(lcdY + viewY) / TILE_WIDTH) * TILES_PER_ROW + 
        viewX / TILE_WIDTH;

    u8 yOffs = (lcdY + viewY) % TILE_WIDTH;

    u8 noWrap = TILES_PER_ROW - viewX / TILE_WIDTH;
    
     // align for whole bytes
    for (short lcdX = 0 - viewX % TILE_WIDTH; lcdX < Screen::LCD_WIDTH; lcdX += TILE_WIDTH, ++tileOffs, --noWrap) 
    { 
        if (!noWrap)
            tileOffs -= TILES_PER_ROW; 
    
        drawBackgroundTile(lcdX, lcdY, memory.read(tileOffs), yOffs, colorIndices, bgArea); 
    }
}

void Ppu::drawWindow() 
{
    int winXx = winX - 7;

    if (winXx < 160 && lcdY >= winY) 
    {
        u8 colorIndices[4] = 
        { 
            static_cast<u8>(bgPalData & 0b11), 
            static_cast<u8>((bgPalData >> 2) & 0b11),
            static_cast<u8>((bgPalData >> 4) & 0b11),
            static_cast<u8>((bgPalData >> 6)),
        };

        bool windowArea = lcdc & 0b00010000;
        bool windowTileArea = lcdc & 0b01000000;

        unsigned short tileOffs = 
            (windowTileArea ? Memory::TILE_M1 : Memory::TILE_M0) +
            (winYCounter / TILE_WIDTH) * TILES_PER_ROW; 

        u8 yOffs = winYCounter % TILE_WIDTH;

        for (u8 lcdX = winX; lcdX < Screen::LCD_WIDTH; lcdX += 8, ++tileOffs) 
        {
            drawBackgroundTile
            (
                lcdX,
                lcdY,
                memory.read(tileOffs),
                yOffs,
                colorIndices,
                windowArea
            );
        }

        ++winYCounter;
    }
}

void Ppu::drawObjectTile
(
    u8 lcdX,
    u8 lcdY,
    u8 tileOffs, 
    u8 yOffs, 
    bool xFlip,
    bool priority,
    u8 *colorIndices
) 
{
    byte notPrioColor;
    if (priority) 
        notPrioColor = COLORS[bgPalData & 0b11];

    word data = memory(Memory::TILES + tileOffs * TILE_BYTE_SIZE + 2 * yOffs);
   
    offs orientation = xFlip ? 1 : -1;
    for (int i = xFlip ? 0 : 7; (i + 1) % 9; i += orientation)
    {
        u8 indexIndex = (data & 0b0001) + ((data >> 7) & 0b0010);
        data >>= 1;
        if (indexIndex && (!priority || (screen(lcdY, lcdX + i) == notPrioColor)))
            screen(lcdY, lcdX + i) = COLORS[colorIndices[indexIndex]];
    }
} 
 
void Ppu::drawObjects() 
{
    bool is16Bit = lcdc & 0b0100; 

    for (int i = numberOfObjects - 1; i >= 0; --i)
    { 
        ObjectLine object = objects[i];

        u8 lcdX = object.posX - 8;
        u8 tileOffs = is16Bit ? object.tileIndex & 0xfe : object.tileIndex;        
 
        bool yFlip = object.attributes & 0b01000000;
        u8 yOffs = yFlip 
            ? (7 + is16Bit * 8 - object.internalY) 
            : object.internalY;

        bool xFlip = object.attributes & 0b00100000;
        bool priority = object.attributes & 0b10000000;

        bool palette = object.attributes & 0b00010000;
        byte objPalData = memory.read(palette ? Memory::OBJ_PAL_1_DATA : Memory::OBJ_PAL_0_DATA);
        u8 colorIndices[4] = 
        { 
            0,
            static_cast<u8>((objPalData >> 2) & 0b11),
            static_cast<u8>((objPalData >> 4) & 0b11),
            static_cast<u8>((objPalData >> 6) & 0b11),
        };

        drawObjectTile
        (
            object.posX - 8, 
            lcdY,
            tileOffs,
            yOffs,
            xFlip,
            priority,
            colorIndices 
        );
    }
}

void Ppu::hBlank() {}

void Ppu::vBlank() 
{
    screen.updateFrame();
    resetWindowY();
    ++lcdY;
}

void Ppu::resetWindowY() { winYCounter = 0; }

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
                    u8 tileOffs = memory.read
                    (
                        (v & 1 ? Memory::TILE_M1 : Memory::TILE_M0) +
                        i * TILES_PER_ROW + k
                    );
                    word data = v & 2 ? 
                        memory(Memory::TILES +               tileOffs * TILE_BYTE_SIZE + 2 * j) : 
                        memory(Memory::TILES + 0x1000 + (offs) tileOffs * TILE_BYTE_SIZE + 2 * j) ;

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

