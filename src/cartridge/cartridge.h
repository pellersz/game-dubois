#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "types.h"
#include <string>

class Cartridge 
{
public: 
    int size;
    byte *rom;

    Cartridge(std::string filename);
    ~Cartridge();

private: 
    long getFileSize(std::string filename); 
};

#endif
