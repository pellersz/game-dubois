#include "mbc.h"
#include "cartridge.h"
#include <iostream>

void Mbc::init(std::shared_ptr<Cartridge> p_cartridge) { this->pCartridge = p_cartridge; }

