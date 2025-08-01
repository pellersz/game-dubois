#include "cpu.h"

unsigned short Cpu::getAF() { return (((short) A) << 8) + F; }

unsigned short Cpu::getBC() { return (((short) B) << 8) + C; }

unsigned short Cpu::getDE() { return (((short) D) << 8) + E; }

unsigned short Cpu::getHL() { return (((short) H) << 8) + L; }

bool Cpu::getZF() { return F & ZF_MASK; }

bool Cpu::getNF() { return F & NF_MASK; }

bool Cpu::getHF() { return F & HF_MASK; }

bool Cpu::getCF() { return F & CF_MASK; }

void Cpu::setAF(unsigned short val) { F = val; A = val >> 8; }

void Cpu::setBC(unsigned short val) { C = val; B = val >> 8; }

void Cpu::setDE(unsigned short val) { E = val; D = val >> 8; }

void Cpu::setHL(unsigned short val) { L = val; H = val >> 8; }

void Cpu::setZF(bool val) { F |= val * 0b0001; } 

void Cpu::setNF(bool val) { F |= val * 0b0010; }

void Cpu::setHF(bool val) { F |= val * 0b0100; }

void Cpu::setCF(bool val) { F |= val * 0b1000; }

// TODO: check if this is where the stack is supposed to go
void Cpu::stackStep() { --SP; }

void Cpu::stackStepBack() { ++SP; }

void Cpu::programCounterStep() { ++PC; }

// TODO: getIF and getIE to implement
unsigned char Cpu::getIF () { return 0; }

unsigned char Cpu::getIE () { return 0; }

void Cpu::executeRegular(unsigned char v) {

}
