#include "cpu.h"

inline unsigned short Cpu::getAF() { return (((short) A) << 8) + F; }

inline unsigned short Cpu::getBC() { return (((short) B) << 8) + C; }

inline unsigned short Cpu::getDE() { return (((short) D) << 8) + E; }

inline unsigned short Cpu::getHL() { return (((short) H) << 8) + L; }

inline bool Cpu::getZF() { return F & 0b0001; }

inline bool Cpu::getNF() { return F & 0b0010; }

inline bool Cpu::getHF() { return F & 0b0100; }

inline bool Cpu::getCF() { return F & 0b1000; }

inline void Cpu::setAF(unsigned short val) { F = val; A = val >> 8; }

inline void Cpu::setBC(unsigned short val) { C = val; B = val >> 8; }

inline void Cpu::setDE(unsigned short val) { E = val; D = val >> 8; }

inline void Cpu::setHL(unsigned short val) { L = val; H = val >> 8; }

inline void Cpu::setZF(bool val) { F |= val * 0b0001; } 

inline void Cpu::setNF(bool val) { F |= val * 0b0010; }

inline void Cpu::setHF(bool val) { F |= val * 0b0100; }

inline void Cpu::setCF(bool val) { F |= val * 0b1000; }


