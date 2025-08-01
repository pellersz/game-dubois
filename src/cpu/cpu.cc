#include "cpu.h"

word Cpu::getAF() { return (((word) A) << 8) + F; }

word Cpu::getBC() { return (((word) B) << 8) + C; }

word Cpu::getDE() { return (((word) D) << 8) + E; }

word Cpu::getHL() { return (((word) H) << 8) + L; }

bool Cpu::getZF() { return F & ZF_MASK; }

bool Cpu::getNF() { return F & NF_MASK; }

bool Cpu::getHF() { return F & HF_MASK; }

bool Cpu::getCF() { return F & CF_MASK; }

void Cpu::setAF(word val) { F = val; A = val >> 8; }

void Cpu::setBC(word val) { C = val; B = val >> 8; }

void Cpu::setDE(word val) { E = val; D = val >> 8; }

void Cpu::setHL(word val) { L = val; H = val >> 8; }

void Cpu::setZF(bool val) { F |= val * 0b0001; } 

void Cpu::setNF(bool val) { F |= val * 0b0010; }

void Cpu::setHF(bool val) { F |= val * 0b0100; }

void Cpu::setCF(bool val) { F |= val * 0b1000; }

// TODO: check if this is where the stack is supposed to go
void Cpu::stackStep() { --SP; }

void Cpu::stackStepBack() { ++SP; }

void Cpu::programCounterStep() { ++PC; }

// TODO: getIF and getIE to implement
byte Cpu::getIF () { return 0; }

byte Cpu::getIE () { return 0; }

void Cpu::executeRegular(byte v) {

}
