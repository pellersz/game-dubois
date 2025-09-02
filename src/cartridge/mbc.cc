#include "mbc.h"
#include "cartridge.h"

void Mbc::init(std::shared_ptr<Cartridge> p_cartridge) 
{ 
    this->p_cartridge = p_cartridge; 
    ramOffs = p_cartridge->getRomSize();
}

