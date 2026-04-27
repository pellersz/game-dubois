#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "types.h"
#include <memory>
#include <string>
#include <vector>

class Memory;
class Mbc;

#define KB 1024 
#define MB 1024 * 1024
#define ROM_BANK_SIZE 0x4000

class Cartridge 
{
  public:
    std::vector<byte> data;

    Cartridge(std::string);
    ~Cartridge();
    void loadRam(std::string);
   
    int getSize();
    int getRomSize();
    int getRamSize();

    byte readBank(unsigned short);
    byte *getBankPointer(unsigned short);
    byte readBankN(unsigned short);
    byte *getBankNPointer(unsigned short);
    byte readRam(unsigned short);
    byte *getRamPointer(unsigned short);
    void writeToRegister(unsigned short, byte);
    void writeToRam(unsigned short, byte);

  private:
    int size;
    int romSize;
    int ramSize = 0;
    bool batteryBacked;

    std::string name;

    std::unique_ptr<Mbc> pMbc;

    long getFileSize(std::string); 
};

#endif
