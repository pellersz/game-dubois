#include "cpu.h"

inline unsigned short Cpu::getDE() {
    return (((short) D) << 8) + E;
}

inline unsigned short Cpu::getHL() {
    return (((short) H) << 8) + L;
}

inline void Cpu::setDE(unsigned short val) {
    E = val;
    D = val >> 8;
}

inline void Cpu::setHL(unsigned short val) {
    E = val;
    D = val >> 8;
}

