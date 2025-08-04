#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "types.h"
#include <string>

class Cartridge {
public: 
    Cartridge(std::string filename);
    void loadCartridge(byte* dist);

private:
    int size;
    byte *rom;

    long getFileSize(std::string filename); 
};

#endif
