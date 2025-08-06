#include "cartridge.h"
#include "types.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>

Cartridge::Cartridge(std::string filename) {
    size = getFileSize(filename);
    if (size == -1)
        throw "Could not get file size";
    rom = new byte[size];
    
    FILE* file = fopen(filename.c_str(), "r");
    if (file == NULL)
        throw "Could not open file for reading";

    fread(rom, 1, size, file);
}

Cartridge::~Cartridge() { delete[] rom; }

long Cartridge::getFileSize(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}
