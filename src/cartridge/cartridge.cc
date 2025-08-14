#include "cartridge.h"
#include "types.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/stat.h>

Cartridge::Cartridge(std::string filename) {
    size = getFileSize(filename);
    if (size == -1) 
    {
        std::cout << "Could not get file size" << std::endl;
        throw 1;
    }
    rom = new byte[size];
    
    FILE* file = fopen(filename.c_str(), "r");
    if (file == NULL)
    {
        std::cout << "Could not open file for reading" << std::endl;
        throw 2;
    }

    fread(rom, 1, size, file); 
}

Cartridge::~Cartridge() { delete[] rom; }

long Cartridge::getFileSize(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}
