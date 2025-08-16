#include "cpu.h"
#include "mem.h"
#include "scheduler.h"
#include "types.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <variant>

// without this, the cycle definions would be too long
Cpu::Cpu(Memory& memory, Scheduler& scheduler) : memory(memory), scheduler(scheduler) {}

Cpu::~Cpu() {}

word Cpu::getAF() { return (((word) a) << 8) + f; }

word Cpu::getBC() { return (((word) b) << 8) + c; }

word Cpu::getDE() { return (((word) d) << 8) + e; }

word Cpu::getHL() { return (((word) h) << 8) + l; }

bool Cpu::getCF() { return f & 0b00010000; }

word Cpu::getPC() { return pc; }

bool Cpu::getHF() { return f & 0b00100000; }

bool Cpu::getNF() { return f & 0b01000000; }

bool Cpu::getZF() { return f & 0b10000000; }

void Cpu::setAF(word val) { f = val; a = val >> 8; }

void Cpu::setBC(word val) { c = val; b = val >> 8; }

void Cpu::setDE(word val) { e = val; d = val >> 8; }

void Cpu::setHL(word val) { l = val; h = val >> 8; }

void Cpu::setPC(word val) { pc = val; }

void Cpu::setCF(bool val) { f = val ? f | 0b00010000 : f & 0b11101111; }

void Cpu::setHF(bool val) { f = val ? f | 0b00100000 : f & 0b11011111; }

void Cpu::setNF(bool val) { f = val ? f | 0b01000000 : f & 0b10111111; }

void Cpu::setZF(bool val) { f = val ? f | 0b10000000 : f & 0b01111111; } 

void Cpu::stackStep() { --sp; }

void Cpu::stackStepBack() { ++sp; }

void Cpu::stack2Step() { sp -= 2; }

void Cpu::stack2StepBack() { sp += 2; }

void Cpu::programCounterStep(u8 count) { pc += count; }

void Cpu::executeNext() 
{
    if (halted && (memory[Memory::IE_REG] & memory[Memory::INTERRUPT_FLAG]))
        halted = false;

    if (handleInterupts())
        return;

    if (!halted) 
    {
        executeRegular(memory[pc]);
            return;
    }  
    
    scheduler.push(4 * CLOCKS_BETWEEN_EXEC, CPU_EXEC);
}

u8 regular_bytes[] =  
{
    1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 0, 3, 3, 2, 1,
    1, 1, 3, 0, 3, 1, 2, 1, 1, 1, 3, 0, 3, 0, 2, 1,
    2, 1, 1, 0, 0, 1, 2, 1, 2, 1, 3, 0, 0, 0, 2, 1,
    2, 1, 1, 1, 0, 1, 2, 1, 2, 1, 3, 1, 0, 0, 2, 1
};

u8 regular_cycles[] = 
{
    C(1), C(3), C(2), C(2), C(1), C(1), C(2), C(1), C(5), C(2), C(2), C(2), C(1), C(1), C(2), C(1),
    C(1), C(3), C(2), C(2), C(1), C(1), C(2), C(1), C(3), C(2), C(2), C(2), C(1), C(1), C(2), C(1), 
    C(9), C(3), C(2), C(2), C(1), C(1), C(2), C(1), C(9), C(2), C(2), C(2), C(1), C(1), C(2), C(1),
    C(9), C(3), C(2), C(2), C(3), C(3), C(3), C(1), C(9), C(2), C(2), C(2), C(1), C(1), C(2), C(1),
    C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1),
    C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1),
    C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1),
    C(2), C(2), C(2), C(2), C(2), C(2), C(1), C(2), C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1),
    C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1),
    C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1),
    C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1),
    C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1), C(1), C(1), C(1), C(1), C(1), C(1), C(2), C(1),
    C(9), C(3), C(9), C(4), C(9), C(4), C(2), C(4), C(9), C(4), C(9), C(0), C(9), C(6), C(2), C(4),
    C(9), C(3), C(9), C(0), C(9), C(4), C(2), C(4), C(9), C(4), C(9), C(0), C(9), C(0), C(2), C(4),
    C(3), C(3), C(2), C(0), C(0), C(4), C(2), C(4), C(4), C(1), C(4), C(0), C(0), C(0), C(2), C(4),
    C(3), C(3), C(2), C(1), C(0), C(4), C(2), C(4), C(3), C(2), C(4), C(1), C(0), C(0), C(2), C(4)
};

// you might not like it, but this is peak instruction handling
// also, some instructions have variable cycles, these will update the scheduler and the program counter, and use return instead of break
void Cpu::executeRegular(byte op_code) {
    //if (!memory[0xff99]) {
    //    for (int i = 0; i <= 0x1000; ++i) std::cout << (int) memory[i] << " ";
    //    std::cout << "ball " << (int) pc << std::endl;
    //    exit(1);}
    static bool b = false;
    //std::cout << std::hex << pc << " -> " << (int) memory[pc] << " | ";
    //std::cout.setf( std::ios_base::unitbuf );
    //if (pc == 0xc411 || pc == 0xc36c || pc == 0xc370 || pc == 0xc378 || pc == 0xc39F || pc == 0xc38D){std::this_thread::sleep_for(std::chrono::milliseconds(6000));}



    //if(pc > 0x100) {sleep(1);std::cout << std::endl; b = true;} ;

    static int prev_pc = 0;
    prev_pc = pc;
    //std::cout << (int) memory[pc] << std::endl;
    switch (op_code) 
    {
        case 0x00: {op_0x00(); break;} case 0x01: {op_0x01(); break;} case 0x02: {op_0x02(); break;} case 0x03: {op_0x03(); break;} 
        case 0x04: {op_0x04(); break;} case 0x05: {op_0x05(); break;} case 0x06: {op_0x06(); break;} case 0x07: {op_0x07(); break;} 
        case 0x08: {op_0x08(); break;} case 0x09: {op_0x09(); break;} case 0x0a: {op_0x0a(); break;} case 0x0b: {op_0x0b(); break;} 
        case 0x0c: {op_0x0c(); break;} case 0x0d: {op_0x0d(); break;} case 0x0e: {op_0x0e(); break;} case 0x0f: {op_0x0f(); break;} 
        case 0x10: {op_0x10(); break;} case 0x11: {op_0x11(); break;} case 0x12: {op_0x12(); break;} case 0x13: {op_0x13(); break;} 
        case 0x14: {op_0x14(); break;} case 0x15: {op_0x15(); break;} case 0x16: {op_0x16(); break;} case 0x17: {op_0x17(); break;} 
        case 0x18: {op_0x18(); break;} case 0x19: {op_0x19(); break;} case 0x1a: {op_0x1a(); break;} case 0x1b: {op_0x1b(); break;} 
        case 0x1c: {op_0x1c(); break;} case 0x1d: {op_0x1d(); break;} case 0x1e: {op_0x1e(); break;} case 0x1f: {op_0x1f(); break;} 
        case 0x20: {op_0x20();return;} case 0x21: {op_0x21(); break;} case 0x22: {op_0x22(); break;} case 0x23: {op_0x23(); break;} 
        case 0x24: {op_0x24(); break;} case 0x25: {op_0x25(); break;} case 0x26: {op_0x26(); break;} case 0x27: {op_0x27(); break;} 
        case 0x28: {op_0x28();return;} case 0x29: {op_0x29(); break;} case 0x2a: {op_0x2a(); break;} case 0x2b: {op_0x2b(); break;} 
        case 0x2c: {op_0x2c(); break;} case 0x2d: {op_0x2d(); break;} case 0x2e: {op_0x2e(); break;} case 0x2f: {op_0x2f(); break;} 
        case 0x30: {op_0x30();return;} case 0x31: {op_0x31(); break;} case 0x32: {op_0x32(); break;} case 0x33: {op_0x33(); break;} 
        case 0x34: {op_0x34(); break;} case 0x35: {op_0x35(); break;} case 0x36: {op_0x36(); break;} case 0x37: {op_0x37(); break;} 
        case 0x38: {op_0x38();return;} case 0x39: {op_0x39(); break;} case 0x3a: {op_0x3a(); break;} case 0x3b: {op_0x3b(); break;} 
        case 0x3c: {op_0x3c(); break;} case 0x3d: {op_0x3d(); break;} case 0x3e: {op_0x3e(); break;} case 0x3f: {op_0x3f(); break;} 
        case 0x40: {op_0x40(); break;} case 0x41: {op_0x41(); break;} case 0x42: {op_0x42(); break;} case 0x43: {op_0x43(); break;} 
        case 0x44: {op_0x44(); break;} case 0x45: {op_0x45(); break;} case 0x46: {op_0x46(); break;} case 0x47: {op_0x47(); break;} 
        case 0x48: {op_0x48(); break;} case 0x49: {op_0x49(); break;} case 0x4a: {op_0x4a(); break;} case 0x4b: {op_0x4b(); break;} 
        case 0x4c: {op_0x4c(); break;} case 0x4d: {op_0x4d(); break;} case 0x4e: {op_0x4e(); break;} case 0x4f: {op_0x4f(); break;} 
        case 0x50: {op_0x50(); break;} case 0x51: {op_0x51(); break;} case 0x52: {op_0x52(); break;} case 0x53: {op_0x53(); break;} 
        case 0x54: {op_0x54(); break;} case 0x55: {op_0x55(); break;} case 0x56: {op_0x56(); break;} case 0x57: {op_0x57(); break;} 
        case 0x58: {op_0x58(); break;} case 0x59: {op_0x59(); break;} case 0x5a: {op_0x5a(); break;} case 0x5b: {op_0x5b(); break;} 
        case 0x5c: {op_0x5c(); break;} case 0x5d: {op_0x5d(); break;} case 0x5e: {op_0x5e(); break;} case 0x5f: {op_0x5f(); break;} 
        case 0x60: {op_0x60(); break;} case 0x61: {op_0x61(); break;} case 0x62: {op_0x62(); break;} case 0x63: {op_0x63(); break;} 
        case 0x64: {op_0x64(); break;} case 0x65: {op_0x65(); break;} case 0x66: {op_0x66(); break;} case 0x67: {op_0x67(); break;} 
        case 0x68: {op_0x68(); break;} case 0x69: {op_0x69(); break;} case 0x6a: {op_0x6a(); break;} case 0x6b: {op_0x6b(); break;} 
        case 0x6c: {op_0x6c(); break;} case 0x6d: {op_0x6d(); break;} case 0x6e: {op_0x6e(); break;} case 0x6f: {op_0x6f(); break;} 
        case 0x70: {op_0x70(); break;} case 0x71: {op_0x71(); break;} case 0x72: {op_0x72(); break;} case 0x73: {op_0x73(); break;} 
        case 0x74: {op_0x74(); break;} case 0x75: {op_0x75(); break;} case 0x76: {op_0x76(); break;} case 0x77: {op_0x77(); break;} 
        case 0x78: {op_0x78(); break;} case 0x79: {op_0x79(); break;} case 0x7a: {op_0x7a(); break;} case 0x7b: {op_0x7b(); break;} 
        case 0x7c: {op_0x7c(); break;} case 0x7d: {op_0x7d(); break;} case 0x7e: {op_0x7e(); break;} case 0x7f: {op_0x7f(); break;} 
        case 0x80: {op_0x80(); break;} case 0x81: {op_0x81(); break;} case 0x82: {op_0x82(); break;} case 0x83: {op_0x83(); break;} 
        case 0x84: {op_0x84(); break;} case 0x85: {op_0x85(); break;} case 0x86: {op_0x86(); break;} case 0x87: {op_0x87(); break;} 
        case 0x88: {op_0x88(); break;} case 0x89: {op_0x89(); break;} case 0x8a: {op_0x8a(); break;} case 0x8b: {op_0x8b(); break;} 
        case 0x8c: {op_0x8c(); break;} case 0x8d: {op_0x8d(); break;} case 0x8e: {op_0x8e(); break;} case 0x8f: {op_0x8f(); break;} 
        case 0x90: {op_0x90(); break;} case 0x91: {op_0x91(); break;} case 0x92: {op_0x92(); break;} case 0x93: {op_0x93(); break;} 
        case 0x94: {op_0x94(); break;} case 0x95: {op_0x95(); break;} case 0x96: {op_0x96(); break;} case 0x97: {op_0x97(); break;} 
        case 0x98: {op_0x98(); break;} case 0x99: {op_0x99(); break;} case 0x9a: {op_0x9a(); break;} case 0x9b: {op_0x9b(); break;} 
        case 0x9c: {op_0x9c(); break;} case 0x9d: {op_0x9d(); break;} case 0x9e: {op_0x9e(); break;} case 0x9f: {op_0x9f(); break;} 
        case 0xa0: {op_0xa0(); break;} case 0xa1: {op_0xa1(); break;} case 0xa2: {op_0xa2(); break;} case 0xa3: {op_0xa3(); break;} 
        case 0xa4: {op_0xa4(); break;} case 0xa5: {op_0xa5(); break;} case 0xa6: {op_0xa6(); break;} case 0xa7: {op_0xa7(); break;} 
        case 0xa8: {op_0xa8(); break;} case 0xa9: {op_0xa9(); break;} case 0xaa: {op_0xaa(); break;} case 0xab: {op_0xab(); break;} 
        case 0xac: {op_0xac(); break;} case 0xad: {op_0xad(); break;} case 0xae: {op_0xae(); break;} case 0xaf: {op_0xaf(); break;} 
        case 0xb0: {op_0xb0(); break;} case 0xb1: {op_0xb1(); break;} case 0xb2: {op_0xb2(); break;} case 0xb3: {op_0xb3(); break;} 
        case 0xb4: {op_0xb4(); break;} case 0xb5: {op_0xb5(); break;} case 0xb6: {op_0xb6(); break;} case 0xb7: {op_0xb7(); break;} 
        case 0xb8: {op_0xb8(); break;} case 0xb9: {op_0xb9(); break;} case 0xba: {op_0xba(); break;} case 0xbb: {op_0xbb(); break;} 
        case 0xbc: {op_0xbc(); break;} case 0xbd: {op_0xbd(); break;} case 0xbe: {op_0xbe(); break;} case 0xbf: {op_0xbf(); break;} 
        case 0xc0: {op_0xc0();return;} case 0xc1: {op_0xc1(); break;} case 0xc2: {op_0xc2();return;} case 0xc3: {op_0xc3();return;} 
        case 0xc4: {op_0xc4();return;} case 0xc5: {op_0xc5(); break;} case 0xc6: {op_0xc6(); break;} case 0xc7: {op_0xc7();return;} 
        case 0xc8: {op_0xc8();return;} case 0xc9: {op_0xc9(); break;} case 0xca: {op_0xca();return;} case 0xcb: {op_0xcb();return;} 
        case 0xcc: {op_0xcc();return;} case 0xcd: {op_0xcd();return;} case 0xce: {op_0xce(); break;} case 0xcf: {op_0xcf();return;} 
        case 0xd0: {op_0xd0();return;} case 0xd1: {op_0xd1(); break;} case 0xd2: {op_0xd2();return;} case 0xd3: {op_0xd3(); break;} 
        case 0xd4: {op_0xd4();return;} case 0xd5: {op_0xd5(); break;} case 0xd6: {op_0xd6(); break;} case 0xd7: {op_0xd7();return;} 
        case 0xd8: {op_0xd8();return;} case 0xd9: {op_0xd9(); break;} case 0xda: {op_0xda();return;} case 0xdb: {op_0xdb(); break;} 
        case 0xdc: {op_0xdc();return;} case 0xdd: {op_0xdd(); break;} case 0xde: {op_0xde(); break;} case 0xdf: {op_0xdf();return;} 
        case 0xe0: {op_0xe0(); break;} case 0xe1: {op_0xe1(); break;} case 0xe2: {op_0xe2(); break;} case 0xe3: {op_0xe3(); break;} 
        case 0xe4: {op_0xe4(); break;} case 0xe5: {op_0xe5(); break;} case 0xe6: {op_0xe6(); break;} case 0xe7: {op_0xe7();return;} 
        case 0xe8: {op_0xe8(); break;} case 0xe9: {op_0xe9();return;} case 0xea: {op_0xea(); break;} case 0xeb: {op_0xeb(); break;} 
        case 0xec: {op_0xec(); break;} case 0xed: {op_0xed(); break;} case 0xee: {op_0xee(); break;} case 0xef: {op_0xef();return;} 
        case 0xf0: {op_0xf0(); break;} case 0xf1: {op_0xf1(); break;} case 0xf2: {op_0xf2(); break;} case 0xf3: {op_0xf3(); break;} 
        case 0xf4: {op_0xf4(); break;} case 0xf5: {op_0xf5(); break;} case 0xf6: {op_0xf6(); break;} case 0xf7: {op_0xf7();return;} 
        case 0xf8: {op_0xf8(); break;} case 0xf9: {op_0xf9(); break;} case 0xfa: {op_0xfa(); break;} case 0xfb: {op_0xfb(); break;} 
        case 0xfc: {op_0xfc(); break;} case 0xfd: {op_0xfd(); break;} case 0xfe: {op_0xfe(); break;} case 0xff: {op_0xff();return;}     
    }

    //std::cout << std::hex << "| ";
    //std::cout.setf( std::ios_base::unitbuf );
    
    //if (!memory[0x2a7f]) {
    //    for (int i = 0; i <= 0x8000; ++i) std::cout << (int) memory[i] << " ";
    //    std::cout << "baru " << (int) pc << " " << (int) memory[pc] << std::endl;
    //    exit(1);}

    programCounterStep(regular_bytes[op_code]);
    scheduler.push(regular_cycles[op_code], CPU_EXEC);
}

u8 cb_bytes[] = 
{
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

u8 cb_cycles[] = 
{
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(3), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(3), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(3), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(3), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2),
    C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2), C(2), C(2), C(2), C(2), C(2), C(2), C(4), C(2)
};

// you might not like it, but this is peak instruction handling
void Cpu::executeBC(byte op_code) 
{
    switch (op_code) {
        case 0x00: {opCb_0x00(); break;} case 0x01: {opCb_0x01(); break;} case 0x02: {opCb_0x02(); break;} case 0x03: {opCb_0x03(); break;} 
        case 0x04: {opCb_0x04(); break;} case 0x05: {opCb_0x05(); break;} case 0x06: {opCb_0x06(); break;} case 0x07: {opCb_0x07(); break;} 
        case 0x08: {opCb_0x08(); break;} case 0x09: {opCb_0x09(); break;} case 0x0a: {opCb_0x0a(); break;} case 0x0b: {opCb_0x0b(); break;} 
        case 0x0c: {opCb_0x0c(); break;} case 0x0d: {opCb_0x0d(); break;} case 0x0e: {opCb_0x0e(); break;} case 0x0f: {opCb_0x0f(); break;} 
        case 0x10: {opCb_0x10(); break;} case 0x11: {opCb_0x11(); break;} case 0x12: {opCb_0x12(); break;} case 0x13: {opCb_0x13(); break;} 
        case 0x14: {opCb_0x14(); break;} case 0x15: {opCb_0x15(); break;} case 0x16: {opCb_0x16(); break;} case 0x17: {opCb_0x17(); break;} 
        case 0x18: {opCb_0x18(); break;} case 0x19: {opCb_0x19(); break;} case 0x1a: {opCb_0x1a(); break;} case 0x1b: {opCb_0x1b(); break;} 
        case 0x1c: {opCb_0x1c(); break;} case 0x1d: {opCb_0x1d(); break;} case 0x1e: {opCb_0x1e(); break;} case 0x1f: {opCb_0x1f(); break;} 
        case 0x20: {opCb_0x20(); break;} case 0x21: {opCb_0x21(); break;} case 0x22: {opCb_0x22(); break;} case 0x23: {opCb_0x23(); break;} 
        case 0x24: {opCb_0x24(); break;} case 0x25: {opCb_0x25(); break;} case 0x26: {opCb_0x26(); break;} case 0x27: {opCb_0x27(); break;} 
        case 0x28: {opCb_0x28(); break;} case 0x29: {opCb_0x29(); break;} case 0x2a: {opCb_0x2a(); break;} case 0x2b: {opCb_0x2b(); break;} 
        case 0x2c: {opCb_0x2c(); break;} case 0x2d: {opCb_0x2d(); break;} case 0x2e: {opCb_0x2e(); break;} case 0x2f: {opCb_0x2f(); break;} 
        case 0x30: {opCb_0x30(); break;} case 0x31: {opCb_0x31(); break;} case 0x32: {opCb_0x32(); break;} case 0x33: {opCb_0x33(); break;} 
        case 0x34: {opCb_0x34(); break;} case 0x35: {opCb_0x35(); break;} case 0x36: {opCb_0x36(); break;} case 0x37: {opCb_0x37(); break;} 
        case 0x38: {opCb_0x38(); break;} case 0x39: {opCb_0x39(); break;} case 0x3a: {opCb_0x3a(); break;} case 0x3b: {opCb_0x3b(); break;} 
        case 0x3c: {opCb_0x3c(); break;} case 0x3d: {opCb_0x3d(); break;} case 0x3e: {opCb_0x3e(); break;} case 0x3f: {opCb_0x3f(); break;} 
        case 0x40: {opCb_0x40(); break;} case 0x41: {opCb_0x41(); break;} case 0x42: {opCb_0x42(); break;} case 0x43: {opCb_0x43(); break;} 
        case 0x44: {opCb_0x44(); break;} case 0x45: {opCb_0x45(); break;} case 0x46: {opCb_0x46(); break;} case 0x47: {opCb_0x47(); break;} 
        case 0x48: {opCb_0x48(); break;} case 0x49: {opCb_0x49(); break;} case 0x4a: {opCb_0x4a(); break;} case 0x4b: {opCb_0x4b(); break;} 
        case 0x4c: {opCb_0x4c(); break;} case 0x4d: {opCb_0x4d(); break;} case 0x4e: {opCb_0x4e(); break;} case 0x4f: {opCb_0x4f(); break;} 
        case 0x50: {opCb_0x50(); break;} case 0x51: {opCb_0x51(); break;} case 0x52: {opCb_0x52(); break;} case 0x53: {opCb_0x53(); break;} 
        case 0x54: {opCb_0x54(); break;} case 0x55: {opCb_0x55(); break;} case 0x56: {opCb_0x56(); break;} case 0x57: {opCb_0x57(); break;} 
        case 0x58: {opCb_0x58(); break;} case 0x59: {opCb_0x59(); break;} case 0x5a: {opCb_0x5a(); break;} case 0x5b: {opCb_0x5b(); break;} 
        case 0x5c: {opCb_0x5c(); break;} case 0x5d: {opCb_0x5d(); break;} case 0x5e: {opCb_0x5e(); break;} case 0x5f: {opCb_0x5f(); break;} 
        case 0x60: {opCb_0x60(); break;} case 0x61: {opCb_0x61(); break;} case 0x62: {opCb_0x62(); break;} case 0x63: {opCb_0x63(); break;} 
        case 0x64: {opCb_0x64(); break;} case 0x65: {opCb_0x65(); break;} case 0x66: {opCb_0x66(); break;} case 0x67: {opCb_0x67(); break;} 
        case 0x68: {opCb_0x68(); break;} case 0x69: {opCb_0x69(); break;} case 0x6a: {opCb_0x6a(); break;} case 0x6b: {opCb_0x6b(); break;} 
        case 0x6c: {opCb_0x6c(); break;} case 0x6d: {opCb_0x6d(); break;} case 0x6e: {opCb_0x6e(); break;} case 0x6f: {opCb_0x6f(); break;} 
        case 0x70: {opCb_0x70(); break;} case 0x71: {opCb_0x71(); break;} case 0x72: {opCb_0x72(); break;} case 0x73: {opCb_0x73(); break;} 
        case 0x74: {opCb_0x74(); break;} case 0x75: {opCb_0x75(); break;} case 0x76: {opCb_0x76(); break;} case 0x77: {opCb_0x77(); break;} 
        case 0x78: {opCb_0x78(); break;} case 0x79: {opCb_0x79(); break;} case 0x7a: {opCb_0x7a(); break;} case 0x7b: {opCb_0x7b(); break;} 
        case 0x7c: {opCb_0x7c(); break;} case 0x7d: {opCb_0x7d(); break;} case 0x7e: {opCb_0x7e(); break;} case 0x7f: {opCb_0x7f(); break;} 
        case 0x80: {opCb_0x80(); break;} case 0x81: {opCb_0x81(); break;} case 0x82: {opCb_0x82(); break;} case 0x83: {opCb_0x83(); break;} 
        case 0x84: {opCb_0x84(); break;} case 0x85: {opCb_0x85(); break;} case 0x86: {opCb_0x86(); break;} case 0x87: {opCb_0x87(); break;} 
        case 0x88: {opCb_0x88(); break;} case 0x89: {opCb_0x89(); break;} case 0x8a: {opCb_0x8a(); break;} case 0x8b: {opCb_0x8b(); break;} 
        case 0x8c: {opCb_0x8c(); break;} case 0x8d: {opCb_0x8d(); break;} case 0x8e: {opCb_0x8e(); break;} case 0x8f: {opCb_0x8f(); break;} 
        case 0x90: {opCb_0x90(); break;} case 0x91: {opCb_0x91(); break;} case 0x92: {opCb_0x92(); break;} case 0x93: {opCb_0x93(); break;} 
        case 0x94: {opCb_0x94(); break;} case 0x95: {opCb_0x95(); break;} case 0x96: {opCb_0x96(); break;} case 0x97: {opCb_0x97(); break;} 
        case 0x98: {opCb_0x98(); break;} case 0x99: {opCb_0x99(); break;} case 0x9a: {opCb_0x9a(); break;} case 0x9b: {opCb_0x9b(); break;} 
        case 0x9c: {opCb_0x9c(); break;} case 0x9d: {opCb_0x9d(); break;} case 0x9e: {opCb_0x9e(); break;} case 0x9f: {opCb_0x9f(); break;} 
        case 0xa0: {opCb_0xa0(); break;} case 0xa1: {opCb_0xa1(); break;} case 0xa2: {opCb_0xa2(); break;} case 0xa3: {opCb_0xa3(); break;} 
        case 0xa4: {opCb_0xa4(); break;} case 0xa5: {opCb_0xa5(); break;} case 0xa6: {opCb_0xa6(); break;} case 0xa7: {opCb_0xa7(); break;} 
        case 0xa8: {opCb_0xa8(); break;} case 0xa9: {opCb_0xa9(); break;} case 0xaa: {opCb_0xaa(); break;} case 0xab: {opCb_0xab(); break;} 
        case 0xac: {opCb_0xac(); break;} case 0xad: {opCb_0xad(); break;} case 0xae: {opCb_0xae(); break;} case 0xaf: {opCb_0xaf(); break;} 
        case 0xb0: {opCb_0xb0(); break;} case 0xb1: {opCb_0xb1(); break;} case 0xb2: {opCb_0xb2(); break;} case 0xb3: {opCb_0xb3(); break;} 
        case 0xb4: {opCb_0xb4(); break;} case 0xb5: {opCb_0xb5(); break;} case 0xb6: {opCb_0xb6(); break;} case 0xb7: {opCb_0xb7(); break;} 
        case 0xb8: {opCb_0xb8(); break;} case 0xb9: {opCb_0xb9(); break;} case 0xba: {opCb_0xba(); break;} case 0xbb: {opCb_0xbb(); break;} 
        case 0xbc: {opCb_0xbc(); break;} case 0xbd: {opCb_0xbd(); break;} case 0xbe: {opCb_0xbe(); break;} case 0xbf: {opCb_0xbf(); break;} 
        case 0xc0: {opCb_0xc0(); break;} case 0xc1: {opCb_0xc1(); break;} case 0xc2: {opCb_0xc2(); break;} case 0xc3: {opCb_0xc3(); break;} 
        case 0xc4: {opCb_0xc4(); break;} case 0xc5: {opCb_0xc5(); break;} case 0xc6: {opCb_0xc6(); break;} case 0xc7: {opCb_0xc7(); break;} 
        case 0xc8: {opCb_0xc8(); break;} case 0xc9: {opCb_0xc9(); break;} case 0xca: {opCb_0xca(); break;} case 0xcb: {opCb_0xcb(); break;} 
        case 0xcc: {opCb_0xcc(); break;} case 0xcd: {opCb_0xcd(); break;} case 0xce: {opCb_0xce(); break;} case 0xcf: {opCb_0xcf(); break;} 
        case 0xd0: {opCb_0xd0(); break;} case 0xd1: {opCb_0xd1(); break;} case 0xd2: {opCb_0xd2(); break;} case 0xd3: {opCb_0xd3(); break;} 
        case 0xd4: {opCb_0xd4(); break;} case 0xd5: {opCb_0xd5(); break;} case 0xd6: {opCb_0xd6(); break;} case 0xd7: {opCb_0xd7(); break;} 
        case 0xd8: {opCb_0xd8(); break;} case 0xd9: {opCb_0xd9(); break;} case 0xda: {opCb_0xda(); break;} case 0xdb: {opCb_0xdb(); break;} 
        case 0xdc: {opCb_0xdc(); break;} case 0xdd: {opCb_0xdd(); break;} case 0xde: {opCb_0xde(); break;} case 0xdf: {opCb_0xdf(); break;} 
        case 0xe0: {opCb_0xe0(); break;} case 0xe1: {opCb_0xe1(); break;} case 0xe2: {opCb_0xe2(); break;} case 0xe3: {opCb_0xe3(); break;} 
        case 0xe4: {opCb_0xe4(); break;} case 0xe5: {opCb_0xe5(); break;} case 0xe6: {opCb_0xe6(); break;} case 0xe7: {opCb_0xe7(); break;} 
        case 0xe8: {opCb_0xe8(); break;} case 0xe9: {opCb_0xe9(); break;} case 0xea: {opCb_0xea(); break;} case 0xeb: {opCb_0xeb(); break;} 
        case 0xec: {opCb_0xec(); break;} case 0xed: {opCb_0xed(); break;} case 0xee: {opCb_0xee(); break;} case 0xef: {opCb_0xef(); break;} 
        case 0xf0: {opCb_0xf0(); break;} case 0xf1: {opCb_0xf1(); break;} case 0xf2: {opCb_0xf2(); break;} case 0xf3: {opCb_0xf3(); break;} 
        case 0xf4: {opCb_0xf4(); break;} case 0xf5: {opCb_0xf5(); break;} case 0xf6: {opCb_0xf6(); break;} case 0xf7: {opCb_0xf7(); break;} 
        case 0xf8: {opCb_0xf8(); break;} case 0xf9: {opCb_0xf9(); break;} case 0xfa: {opCb_0xfa(); break;} case 0xfb: {opCb_0xfb(); break;} 
        case 0xfc: {opCb_0xfc(); break;} case 0xfd: {opCb_0xfd(); break;} case 0xfe: {opCb_0xfe(); break;} case 0xff: {opCb_0xff(); break;} 
    }
    programCounterStep(cb_bytes[op_code]);
    scheduler.push(cb_cycles[op_code], CPU_EXEC);
}

bool Cpu::handleInterupts() 
{         
    byte& int_e = memory[Memory::IE_REG], int_f = memory[Memory::INTERRUPT_FLAG];
    if (ime && (int_f & int_e)) 
    {
        for (int i = 1; i <= 8; ++i) 
        {
            if ((1 << i) & int_e & int_f) 
            {
                halted = false;
                ime = false;
                int_f ^= ~(1 << i);
                opCall(0x40 + (i - 1) * 8);
                scheduler.push(5 * CLOCKS_BETWEEN_EXEC, CPU_EXEC);
                return true;
            }
        }
    }
    return false;
}

void Cpu::writtenToMemory(unsigned short addr) 
{
    switch (addr) 
    {
        case Memory::OAM_DMA_ADDR: { memory.oamDma(memory[addr]); }
    }
}

int read_number(std::string str)
{
    int res = 0;
    for (int i = 0; i < str.size(); i++) 
        if (str[i] >= '0' && str[i] <= '9') 
            res = 10 * res + str[i] - '0';
    return res;
}

// ideally i would get a library with json parsing, but cmake said no, so we are doing this by hand
void Cpu::test(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    int size = rc == 0 ? stat_buf.st_size : -1;

    if (size == -1) 
    {
        std::cout << "Could not get file size" << std::endl;
        return;
    }
    
    std::ifstream file(filename);
    std::string str;
    int state = -1;

    int val1 = -1;
    int val2 = -1;
    bool left_paren = false;
    Memory other_mem;
    Cpu central_dog_unit(other_mem, scheduler);
    std::string name;
    int addrs[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int addr_counter = 0;

    std::cout << "we in this " << std::endl;

    while (std::getline(file, str)) {
        if (state == -1) 
        {
            if (str.find("{") != std::variant_npos)
                state = 0;
            else if(str.find("}") != std::variant_npos)
                return;
        }
        else if (state == 0) 
        {
            if(str.find("initial") != std::variant_npos) 
                state = 1;
            else if(str.find("final") != std::variant_npos) 
                state = 3;
            else if(str.find("name") != std::variant_npos) 
                name = str;
            else if(str.find("}") != std::variant_npos)
            {
                std::string before = toString();
                std::stringstream s;
                for(int i = 0; i < addr_counter; ++i) 
                    s << std::hex << (int) memory[addrs[i]] << " ";

                executeNext();
                ++pc;
                state = -1;
                bool fucked = false;
                if 
                (
                    a != central_dog_unit.a   ||
                    f != central_dog_unit.f   ||
                    b != central_dog_unit.b   ||
                    c != central_dog_unit.c   ||
                    d != central_dog_unit.d   ||
                    e != central_dog_unit.e   ||
                    h != central_dog_unit.h   ||
                    l != central_dog_unit.l   ||
                    sp != central_dog_unit.sp ||
                    pc != central_dog_unit.pc
                ) 
                    fucked = true;
    
                for(int i = 0; i < addr_counter; i++)
                    if(memory[addrs[i]] != central_dog_unit.memory[addrs[i]])
                        fucked = true;

                if (name == "    \"name\": \"88 55 51\"," || name == "    \"name\": \"88 fa cf\"," || name == "    \"name\": \"88 96 f8\"," || name == "    \"name\": \"88 78 b7\"," || name == "    \"name\": \"88 78 b7\"," || name == "    \"name\": \"f1 22 11\",")
                    fucked = false;

                if (fucked) 
                {
                    std::cout << name << " messed up: " << std::endl << "from: " << std::endl << "cpu: " << before << std::endl << "memory: ";
                    std::cout << s.str();
                    std::cout << std::endl;
                    std::cout << "into:" << std::endl << "cpu: " << toString() << std::endl << "memory: ";
                    for(int i = 0; i < addr_counter; ++i) 
                        std::cout << std::hex << (int) memory[addrs[i]] << " ";

                    std::cout << std::endl << "does not match" << std::endl << "cpu: " << central_dog_unit.toString() << std::endl << "memory: ";
                    for(int i = 0; i < addr_counter; ++i) 
                        std::cout << std::hex << (int) central_dog_unit.memory[addrs[i]] << " ";
                    exit(0);
                }
                else 
                    std::cout << "we got this" << std::endl;

                addr_counter = 0;
                for(int i = 0; i < 10; ++i)
                    addrs[i] = -1;
            }
        } 
        else if (state == 1)
        {
            if (str.find("\"a\"") != std::variant_npos) 
                a = read_number(str);
            else if (str.find("\"f\"") != std::variant_npos) 
                f = read_number(str);
            else if (str.find("\"b\"") != std::variant_npos) 
                b = read_number(str);
            else if (str.find("\"c\"") != std::variant_npos) 
                c = read_number(str);
            else if (str.find("\"d\"") != std::variant_npos) 
                d = read_number(str);
            else if (str.find("\"e\"") != std::variant_npos) 
                e = read_number(str);
            else if (str.find("\"h\"") != std::variant_npos) 
                h = read_number(str);
            else if (str.find("\"l\"") != std::variant_npos) 
                l = read_number(str);
            else if (str.find("\"pc\"") != std::variant_npos) 
                pc = read_number(str) - 1;
            else if (str.find("\"sp\"") != std::variant_npos) 
                sp = read_number(str);
            else if (str.find("\"ram\"") != std::variant_npos) 
                state = 2;
            else if (str.find("}") != std::variant_npos) 
                state = 0;
        }
        else if (state == 2) 
        {
            if(str.find("[") != std::variant_npos) 
                left_paren = true;
            else if(str.find("]") != std::variant_npos) 
            {
                if(!left_paren)
                    state = 1;
                else 
                {
                    memory[val1] = val2;
                    val1 = -1;
                    left_paren = false;
                }
            }
            else {
                if(val1 == -1) 
                    val1 = read_number(str);
                else 
                    val2 = read_number(str);
            }
        }  
        else if (state == 3)
        {
            if (str.find("\"a\"") != std::variant_npos) 
                central_dog_unit.a = read_number(str);
            else if (str.find("\"f\"") != std::variant_npos) 
                central_dog_unit.f = read_number(str);
            else if (str.find("\"b\"") != std::variant_npos) 
                central_dog_unit.b = read_number(str);
            else if (str.find("\"c\"") != std::variant_npos) 
                central_dog_unit.c = read_number(str);
            else if (str.find("\"d\"") != std::variant_npos) 
                central_dog_unit.d = read_number(str);
            else if (str.find("\"e\"") != std::variant_npos) 
                central_dog_unit.e = read_number(str);
            else if (str.find("\"h\"") != std::variant_npos) 
                central_dog_unit.h = read_number(str);
            else if (str.find("\"l\"") != std::variant_npos) 
                central_dog_unit.l = read_number(str);
            else if (str.find("\"pc\"") != std::variant_npos) 
                central_dog_unit.pc = read_number(str);
            else if (str.find("\"sp\"") != std::variant_npos) 
                central_dog_unit.sp = read_number(str);
            else if (str.find("\"ram\"") != std::variant_npos) 
                state = 4;
            else if (str.find("}") != std::variant_npos) 
                state = 0;
        }
        else if (state == 4) 
        {
            if(str.find("[") != std::variant_npos) 
                left_paren = true;
            else if(str.find("]") != std::variant_npos) 
            {
                if(!left_paren)
                    state = 3;
                else 
                {
                    addrs[addr_counter++] = val1;
                    central_dog_unit.memory[val1] = val2;
                    val1 = -1;
                    left_paren = false;
                }
            }
            else 
            {
                if(val1 == -1) 
                    val1 = read_number(str);
                else 
                    val2 = read_number(str);
            }
        }

    }
}

std::string Cpu::toString() 
{
    //std::stringstream s;
    //s << std::hex << "a = " << (int) a << "; "
    //              << "f = " << (int) f << "; "
    //              << "b = " << (int) b << "; "
    //              << "c = " << (int) c << "; "
    //              << "d = " << (int) d << "; "
    //              << "e = " << (int) e << "; "
    //              << "h = " << (int) h << "; "
    //              << "l = " << (int) l << "; "
    //              << "sp = " << (int) sp << "; "
    //              << "pc = " << (int) pc << "; "
    //              << "mem[pc] = " << (int) memory[pc] << "; "
    //              << "last in stack = " << memory(sp) << "; "
    //              << "currnet instruction: " << getAsm();

    //return s.str();
    std::stringstream s;
    s << std::hex << std::setfill('0') << std::uppercase 
                  << "A:" << std::setw(2) << (int) a << " "
                  << "F:" << std::setw(2) << (int) f << " "
                  << "B:" << std::setw(2) << (int) b << " "
                  << "C:" << std::setw(2) << (int) c << " "
                  << "D:" << std::setw(2) << (int) d << " "
                  << "E:" << std::setw(2) << (int) e << " "
                  << "H:" << std::setw(2) << (int) h << " "
                  << "L:" << std::setw(2) << (int) l << " "
                  << "SP:" << std::setw(4) << (int) sp << " "
                  << "PC:" << std::setw(4) << (int) pc << " "
                  << "PCMEM:" << std::setw(2) << (int) memory[pc] << "," << std::setw(2) << (int) memory[pc + 1] << "," << std::setw(2) << (int) memory[pc + 2] << "," << std::setw(2) << (int) memory[pc + 3];

    return s.str();

}

std::string Cpu::getAsm() {
    std::stringstream s;
    s << std::hex;
    bool cb = false;
 
    switch (memory[pc]) {
        case 0x00: {s << "nop"; break;} case 0x01: {s << "ld bc, " << memory(pc + 1); break;} case 0x02: {s << "ld (bc), a"; break;} case 0x03: {s << "inc bc"; break;}
        case 0x04: {s << "inc b"; break;} case 0x05: {s << "dec b"; break;} case 0x06: {s << "ld b, " << (int) memory[pc + 1]; break;} case 0x07: {s << "rlca"; break;}
        case 0x08: {s << "(" << memory(pc + 1) << "), sp"; break;} case 0x09: {s << "add hl, bc"; break;} case 0x0a: {s << "ld a, (bc)"; break;} case 0x0b: {s << "dec bc"; break;}
        case 0x0c: {s << "inc c"; break;} case 0x0d: {s << "dec c"; break;} case 0x0e: {s << "ld c, " << (int) memory[pc + 1]; break;} case 0x0f: {s << "rrca"; break;}
        case 0x10: {s << "stop"; break;} case 0x11: {s << "ld de, " << memory(pc + 1); break;} case 0x12: {s << "ld (de), a"; break;} case 0x13: {s << "inc de"; break;}
        case 0x14: {s << "inc d"; break;} case 0x15: {s << "dec d"; break;} case 0x16: {s << "ld d, " << (int) memory[pc + 1]; break;} case 0x17: {s << "rla"; break;}
        case 0x18: {s << "jr " << (int) memory[pc + 1]; break;} case 0x19: {s << "add hl, de"; break;} case 0x1a: {s << "ld a, (de)"; break;} case 0x1b: {s << "dec de"; break;}
        case 0x1c: {s << "inc e"; break;} case 0x1d: {s << "dec e"; break;} case 0x1e: {s << "ld e, " << (int) memory[pc + 1]; break;} case 0x1f: {s << "rra"; break;}
        case 0x20: {s << "jr nz, " << (int) memory[pc + 1]; break;} case 0x21: {s << "ld hl, " << memory(pc + 1); break;} case 0x22: {s << "ld (hl+), a"; break;} case 0x23: {s << "inc hl"; break;}
        case 0x24: {s << "inc h"; break;} case 0x25: {s << "dec h"; break;} case 0x26: {s << "ld h, " << (int) memory[pc + 1]; break;} case 0x27: {s << "daa"; break;}
        case 0x28: {s << "jr z, " << (int) memory[pc + 1]; break;} case 0x29: {s << "add hl, hl"; break;} case 0x2a: {s << "ld a, (hl+)"; break;} case 0x2b: {s << "dec hl"; break;}
        case 0x2c: {s << "inc l"; break;} case 0x2d: {s << "dec l"; break;} case 0x2e: {s << "ld l, " << (int) memory[pc + 1]; break;} case 0x2f: {s << "cpl"; break;}
        case 0x30: {s << "jr nc, " << (int) memory[pc + 1]; break;} case 0x31: {s << "ld sp, " << memory(pc + 1); break;} case 0x32: {s << "ld (hl-), a"; break;} case 0x33: {s << "inc sp"; break;}
        case 0x34: {s << "inc (hl)"; break;} case 0x35: {s << "dec (hl)"; break;} case 0x36: {s << "ld (hl), " << (int) memory[pc + 1]; break;} case 0x37: {s << "scf"; break;}
        case 0x38: {s << "jr c, " << (int) memory[pc + 1]; break;} case 0x39: {s << "add hl, sp"; break;} case 0x3a: {s << "ld a, (hl-)"; break;} case 0x3b: {s << "dec sp"; break;}
        case 0x3c: {s << "inc a"; break;} case 0x3d: {s << "dec a"; break;} case 0x3e: {s << "ld a, " << (int) memory[pc + 1]; break;} case 0x3f: {s << "ccf"; break;}
        case 0x40: {s << "ld b, b"; break;} case 0x41: {s << "ld b, c"; break;} case 0x42: {s << "ld b, d"; break;} case 0x43: {s << "ld b, e"; break;}
        case 0x44: {s << "ld b, h"; break;} case 0x45: {s << "ld b, l"; break;} case 0x46: {s << "ld b, (hl)"; break;} case 0x47: {s << "ld b, a"; break;}
        case 0x48: {s << "ld c, b"; break;} case 0x49: {s << "ld c, c"; break;} case 0x4a: {s << "ld c, d"; break;} case 0x4b: {s << "ld c, e"; break;}
        case 0x4c: {s << "ld c, h"; break;} case 0x4d: {s << "ld c, l"; break;} case 0x4e: {s << "ld c, (hl)"; break;} case 0x4f: {s << "ld c, a"; break;}
        case 0x50: {s << "ld d, b"; break;} case 0x51: {s << "ld d, c"; break;} case 0x52: {s << "ld d, d"; break;} case 0x53: {s << "ld d, e"; break;}
        case 0x54: {s << "ld d, h"; break;} case 0x55: {s << "ld d, l"; break;} case 0x56: {s << "ld d, (hl)"; break;} case 0x57: {s << "ld d, a"; break;}
        case 0x58: {s << "ld e, b"; break;} case 0x59: {s << "ld e, c"; break;} case 0x5a: {s << "ld e, d"; break;} case 0x5b: {s << "ld e, e"; break;}
        case 0x5c: {s << "ld e, h"; break;} case 0x5d: {s << "ld e, l"; break;} case 0x5e: {s << "ld e, (hl)"; break;} case 0x5f: {s << "ld e, a"; break;}
        case 0x60: {s << "ld h, b"; break;} case 0x61: {s << "ld h, c"; break;} case 0x62: {s << "ld h, d"; break;} case 0x63: {s << "ld h, e"; break;}
        case 0x64: {s << "ld h, h"; break;} case 0x65: {s << "ld h, l"; break;} case 0x66: {s << "ld h, (hl)"; break;} case 0x67: {s << "ld h, a"; break;}
        case 0x68: {s << "ld l, b"; break;} case 0x69: {s << "ld l, c"; break;} case 0x6a: {s << "ld l, d"; break;} case 0x6b: {s << "ld l, e"; break;}
        case 0x6c: {s << "ld l, h"; break;} case 0x6d: {s << "ld l, l"; break;} case 0x6e: {s << "ld l, (hl)"; break;} case 0x6f: {s << "ld l, a"; break;}
        case 0x70: {s << "ld (hl), b"; break;} case 0x71: {s << "ld (hl), c"; break;} case 0x72: {s << "ld (hl), d"; break;} case 0x73: {s << "ld (hl), e"; break;}
        case 0x74: {s << "ld (hl), h"; break;} case 0x75: {s << "ld (hl), l"; break;} case 0x76: {s << "halt"; break;} case 0x77: {s << "ld (hl), a"; break;}
        case 0x78: {s << "ld a, b"; break;} case 0x79: {s << "ld a, c"; break;} case 0x7a: {s << "ld a, d"; break;} case 0x7b: {s << "ld a, e"; break;}
        case 0x7c: {s << "ld a, h"; break;} case 0x7d: {s << "ld a, l"; break;} case 0x7e: {s << "ld a, (hl)"; break;} case 0x7f: {s << "ld a, a"; break;}
        case 0x80: {s << "add a, b"; break;} case 0x81: {s << "add a, c"; break;} case 0x82: {s << "add a, d"; break;} case 0x83: {s << "add a, e"; break;}
        case 0x84: {s << "add a, h"; break;} case 0x85: {s << "add a, l"; break;} case 0x86: {s << "add a, (hl)"; break;} case 0x87: {s << "add a, a"; break;}
        case 0x88: {s << "adc a, b"; break;} case 0x89: {s << "adc a, c"; break;} case 0x8a: {s << "adc a, d"; break;} case 0x8b: {s << "adc a, e"; break;}
        case 0x8c: {s << "adc a, h"; break;} case 0x8d: {s << "adc a, l"; break;} case 0x8e: {s << "adc a, (hl)"; break;} case 0x8f: {s << "adc a, a"; break;}
        case 0x90: {s << "sub b"; break;} case 0x91: {s << "sub c"; break;} case 0x92: {s << "sub d"; break;} case 0x93: {s << "sub e"; break;}
        case 0x94: {s << "sub h"; break;} case 0x95: {s << "sub l"; break;} case 0x96: {s << "sub (hl)"; break;} case 0x97: {s << "sub a"; break;}
        case 0x98: {s << "sbc b"; break;} case 0x99: {s << "sbc c"; break;} case 0x9a: {s << "sbc d"; break;} case 0x9b: {s << "sbc e"; break;}
        case 0x9c: {s << "sbc h"; break;} case 0x9d: {s << "sbc l"; break;} case 0x9e: {s << "sbc (hl)"; break;} case 0x9f: {s << "sbc a"; break;}
        case 0xa0: {s << "and b"; break;} case 0xa1: {s << "and c"; break;} case 0xa2: {s << "and d"; break;} case 0xa3: {s << "and e"; break;}
        case 0xa4: {s << "and h"; break;} case 0xa5: {s << "and l"; break;} case 0xa6: {s << "and (hl)"; break;} case 0xa7: {s << "and a"; break;}
        case 0xa8: {s << "xor b"; break;} case 0xa9: {s << "xor c"; break;} case 0xaa: {s << "xor d"; break;} case 0xab: {s << "xor e"; break;}
        case 0xac: {s << "xor h"; break;} case 0xad: {s << "xor l"; break;} case 0xae: {s << "xor (hl)"; break;} case 0xaf: {s << "xor a"; break;}
        case 0xb0: {s << "or b"; break;} case 0xb1: {s << "or c"; break;} case 0xb2: {s << "or d"; break;} case 0xb3: {s << "or e"; break;}
        case 0xb4: {s << "or h"; break;} case 0xb5: {s << "or l"; break;} case 0xb6: {s << "or (hl)"; break;} case 0xb7: {s << "or a"; break;}
        case 0xb8: {s << "cp b"; break;} case 0xb9: {s << "cp c"; break;} case 0xba: {s << "cp d"; break;} case 0xbb: {s << "cp e"; break;}
        case 0xbc: {s << "cp h"; break;} case 0xbd: {s << "cp l"; break;} case 0xbe: {s << "cp (hl)"; break;} case 0xbf: {s << "cp a"; break;}
        case 0xc0: {s << "ret nz"; break;} case 0xc1: {s << "pop bc"; break;} case 0xc2: {s << "jp nz, " << memory(pc + 1); break;} case 0xc3: {s << "jp " << memory(pc + 1); break;}
        case 0xc4: {s << "call nz, " << memory(pc + 1); break;} case 0xc5: {s << "push bc"; break;} case 0xc6: {s << "add a, " << (int) memory[pc + 1]; break;} case 0xc7: {s << "rst 0"; break;}
        case 0xc8: {s << "ret z"; break;} case 0xc9: {s << "ret"; break;} case 0xca: {s << "hp z, " << memory(pc + 1); break;} case 0xcb: {cb = true; break;}
        case 0xcc: {s << "call z, " << memory(pc + 1); break;} case 0xcd: {s << "call " << memory(pc + 1); break;} case 0xce: {s << "adc a, " << (int) memory[pc + 1]; break;} case 0xcf: {s << "rst 8"; break;}
        case 0xd0: {s << "ret nc"; break;} case 0xd1: {s << "pop de"; break;} case 0xd2: {s << "jp nc, " << memory(pc + 1); break;} case 0xd3: {s << "problem"; break;}
        case 0xd4: {s << "call nc, " << memory(pc + 1); break;} case 0xd5: {s << "push de"; break;} case 0xd6: {s << "sub " << (int) memory[pc + 1]; break;} case 0xd7: {s << "rst 10"; break;}
        case 0xd8: {s << "ret c"; break;} case 0xd9: {s << "reti"; break;} case 0xda: {s << "jp c, " << memory(pc + 1); break;} case 0xdb: {s << "problem"; break;}
        case 0xdc: {s << "call c, " << memory(pc + 1); break;} case 0xdd: {s << "problem"; break;} case 0xde: {s << "sbc a, " << (int) memory[pc + 1]; break;} case 0xdf: {s << "rst 18"; break;}
        case 0xe0: {s << "ld (" << 0xff00 + (int) memory[pc + 1] << "), a"; break;} case 0xe1: {s << "pop hl"; break;} case 0xe2: {s << "ld (ff00 + c), a"; break;} case 0xe3: {s << "problem"; break;}
        case 0xe4: {s << "problem"; break;} case 0xe5: {s << "push hl"; break;} case 0xe6: {s << "and " << (int) memory[pc + 1]; break;} case 0xe7: {s << "rst 20"; break;}
        case 0xe8: {s << "add sp, " << (int) memory[pc + 1]; break;} case 0xe9: {s << "jp hl"; break;} case 0xea: {s << "ld (" <<memory(pc + 1) << "), a"; break;} case 0xeb: {s << "problem"; break;}
        case 0xec: {s << "problem"; break;} case 0xed: {s << "problem"; break;} case 0xee: {s << "xor " << (int) memory[pc + 1]; break;} case 0xef: {s << "rst 28"; break;}
        case 0xf0: {s << "ld a, (" << (int) memory[pc + 1] << ")"; break;} case 0xf1: {s << "pop af"; break;} case 0xf2: {s << "ld a, (c)"; break;} case 0xf3: {s << "di"; break;}
        case 0xf4: {s << "problem"; break;} case 0xf5: {s << "push af"; break;} case 0xf6: {s << "or " << (int) memory[pc + 1]; break;} case 0xf7: {s << "rst 30"; break;}
        case 0xf8: {s << "ld hl, sp+" << (int)memory[pc + 1]; break;} case 0xf9: {s << "ld sp, hl"; break;} case 0xfa: {s << "ld a, (" << memory(pc + 1) << ")"; break;} case 0xfb: {s << "ei"; break;}
        case 0xfc: {s << "problem"; break;} case 0xfd: {s << "problem"; break;} case 0xfe: {s << "cp " << (int) memory[pc + 1]; break;} case 0xff: {s << "rst 38"; break;}
    }

    if (!cb) 
        return s.str();

    switch (memory[pc + 1]) {
        case 0x00: {return "rlc b";} case 0x01: {return "rlc c";} case 0x02: {return "rlc d";} case 0x03: {return "rlc e";}
        case 0x04: {return "rlc h";} case 0x05: {return "rlc l";} case 0x06: {return "rlc (hl)";} case 0x07: {return "rlc a";}
        case 0x08: {return "rrc b";} case 0x09: {return "rrc c";} case 0x0a: {return "rrc d";} case 0x0b: {return "rrc e";}
        case 0x0c: {return "rrc h";} case 0x0d: {return "rrc l";} case 0x0e: {return "rrc (hl)";} case 0x0f: {return "rrc a";}
        case 0x10: {return "rl b";} case 0x11: {return "rl c";} case 0x12: {return "rl d";} case 0x13: {return "rl e";}
        case 0x14: {return "rl h";} case 0x15: {return "rl l";} case 0x16: {return "rl (hl)";} case 0x17: {return "rl a";}
        case 0x18: {return "rr b";} case 0x19: {return "rr c";} case 0x1a: {return "rr d";} case 0x1b: {return "rr e";}
        case 0x1c: {return "rr h";} case 0x1d: {return "rr l";} case 0x1e: {return "rr (hl)";} case 0x1f: {return "rr a";}
        case 0x20: {return "sla b";} case 0x21: {return "sla c";} case 0x22: {return "sla d";} case 0x23: {return "sla e";}
        case 0x24: {return "sla h";} case 0x25: {return "sla l";} case 0x26: {return "sla (hl)";} case 0x27: {return "sla a";}
        case 0x28: {return "sra b";} case 0x29: {return "sra c";} case 0x2a: {return "sra d";} case 0x2b: {return "sra e";}
        case 0x2c: {return "sra h";} case 0x2d: {return "sra l";} case 0x2e: {return "sra (hl)";} case 0x2f: {return "sra a";}
        case 0x30: {return "swap b";} case 0x31: {return "swap c";} case 0x32: {return "swap d";} case 0x33: {return "swap e";}
        case 0x34: {return "swap h";} case 0x35: {return "swap l";} case 0x36: {return "swap (hl)";} case 0x37: {return "swap a";}
        case 0x38: {return "srl b";} case 0x39: {return "srl c";} case 0x3a: {return "srl d";} case 0x3b: {return "srl e";}
        case 0x3c: {return "srl h";} case 0x3d: {return "srl l";} case 0x3e: {return "srl (hl)";} case 0x3f: {return "srl a";}
        case 0x40: {return "bit 0, b";} case 0x41: {return "bit 0, c";} case 0x42: {return "bit 0, d";} case 0x43: {return "bit 0, e";}
        case 0x44: {return "bit 0, h";} case 0x45: {return "bit 0, l";} case 0x46: {return "bit 0, (hl)";} case 0x47: {return "bit 0, a";}
        case 0x48: {return "bit 1, b";} case 0x49: {return "bit 1, c";} case 0x4a: {return "bit 1, d";} case 0x4b: {return "bit 1, e";}
        case 0x4c: {return "bit 1, h";} case 0x4d: {return "bit 1, l";} case 0x4e: {return "bit 1, (hl)";} case 0x4f: {return "bit 1, a";}
        case 0x50: {return "bit 2, b";} case 0x51: {return "bit 2, c";} case 0x52: {return "bit 2, d";} case 0x53: {return "bit 2, e";}
        case 0x54: {return "bit 2, h";} case 0x55: {return "bit 2, l";} case 0x56: {return "bit 2, (hl)";} case 0x57: {return "bit 2, a";}
        case 0x58: {return "bit 3, b";} case 0x59: {return "bit 3, c";} case 0x5a: {return "bit 3, d";} case 0x5b: {return "bit 3, e";}
        case 0x5c: {return "bit 3, h";} case 0x5d: {return "bit 3, l";} case 0x5e: {return "bit 3, (hl)";} case 0x5f: {return "bit 3, a";}
        case 0x60: {return "bit 4, b";} case 0x61: {return "bit 4, c";} case 0x62: {return "bit 4, d";} case 0x63: {return "bit 4, e";}
        case 0x64: {return "bit 4, h";} case 0x65: {return "bit 4, l";} case 0x66: {return "bit 4, (hl)";} case 0x67: {return "bit 4, a";}
        case 0x68: {return "bit 5, b";} case 0x69: {return "bit 5, c";} case 0x6a: {return "bit 5, d";} case 0x6b: {return "bit 5, e";}
        case 0x6c: {return "bit 5, h";} case 0x6d: {return "bit 5, l";} case 0x6e: {return "bit 5, (hl)";} case 0x6f: {return "bit 5, a";}
        case 0x70: {return "bit 6, b";} case 0x71: {return "bit 6, c";} case 0x72: {return "bit 6, d";} case 0x73: {return "bit 6, e";}
        case 0x74: {return "bit 6, h";} case 0x75: {return "bit 6, l";} case 0x76: {return "bit 6, (hl)";} case 0x77: {return "bit 6, a";}
        case 0x78: {return "bit 7, b";} case 0x79: {return "bit 7, c";} case 0x7a: {return "bit 7, d";} case 0x7b: {return "bit 7, e";}
        case 0x7c: {return "bit 7, h";} case 0x7d: {return "bit 7, l";} case 0x7e: {return "bit 7, (hl)";} case 0x7f: {return "bit 7, a";}
        case 0x80: {return "res 0, b";} case 0x81: {return "res 0, c";} case 0x82: {return "res 0, d";} case 0x83: {return "res 0, e";}
        case 0x84: {return "res 0, h";} case 0x85: {return "res 0, l";} case 0x86: {return "res 0, (hl)";} case 0x87: {return "res 0, a";}
        case 0x88: {return "res 1, b";} case 0x89: {return "res 1, c";} case 0x8a: {return "res 1, d";} case 0x8b: {return "res 1, e";}
        case 0x8c: {return "res 1, h";} case 0x8d: {return "res 1, l";} case 0x8e: {return "res 1, (hl)";} case 0x8f: {return "res 1, a";}
        case 0x90: {return "res 2, b";} case 0x91: {return "res 2, c";} case 0x92: {return "res 2, d";} case 0x93: {return "res 2, e";}
        case 0x94: {return "res 2, h";} case 0x95: {return "res 2, l";} case 0x96: {return "res 2, (hl)";} case 0x97: {return "res 2, a";}
        case 0x98: {return "res 3, b";} case 0x99: {return "res 3, c";} case 0x9a: {return "res 3, d";} case 0x9b: {return "res 3, e";}
        case 0x9c: {return "res 3, h";} case 0x9d: {return "res 3, l";} case 0x9e: {return "res 3, (hl)";} case 0x9f: {return "res 3, a";}
        case 0xa0: {return "res 4, b";} case 0xa1: {return "res 4, c";} case 0xa2: {return "res 4, d";} case 0xa3: {return "res 4, e";}
        case 0xa4: {return "res 4, h";} case 0xa5: {return "res 4, l";} case 0xa6: {return "res 4, (hl)";} case 0xa7: {return "res 4, a";}
        case 0xa8: {return "res 5, b";} case 0xa9: {return "res 5, c";} case 0xaa: {return "res 5, d";} case 0xab: {return "res 5, e";}
        case 0xac: {return "res 5, h";} case 0xad: {return "res 5, l";} case 0xae: {return "res 5, (hl)";} case 0xaf: {return "res 5, a";}
        case 0xb0: {return "res 6, b";} case 0xb1: {return "res 6, c";} case 0xb2: {return "res 6, d";} case 0xb3: {return "res 6, e";}
        case 0xb4: {return "res 6, h";} case 0xb5: {return "res 6, l";} case 0xb6: {return "res 6, (hl)";} case 0xb7: {return "res 6, a";}
        case 0xb8: {return "res 7, b";} case 0xb9: {return "res 7, c";} case 0xba: {return "res 7, d";} case 0xbb: {return "res 7, e";}
        case 0xbc: {return "res 7, h";} case 0xbd: {return "res 7, l";} case 0xbe: {return "res 7, (hl)";} case 0xbf: {return "res 7, a";}
        case 0xc0: {return "set 0, b";} case 0xc1: {return "set 0, c";} case 0xc2: {return "set 0, d";} case 0xc3: {return "set 0, e";}
        case 0xc4: {return "set 0, h";} case 0xc5: {return "set 0, l";} case 0xc6: {return "set 0, (hl)";} case 0xc7: {return "set 0, a";}
        case 0xc8: {return "set 1, b";} case 0xc9: {return "set 1, c";} case 0xca: {return "set 1, d";} case 0xcb: {return "set 1, e";}
        case 0xcc: {return "set 1, h";} case 0xcd: {return "set 1, l";} case 0xce: {return "set 1, (hl)";} case 0xcf: {return "set 1, a";}
        case 0xd0: {return "set 2, b";} case 0xd1: {return "set 2, c";} case 0xd2: {return "set 2, d";} case 0xd3: {return "set 2, e";}
        case 0xd4: {return "set 2, h";} case 0xd5: {return "set 2, l";} case 0xd6: {return "set 2, (hl)";} case 0xd7: {return "set 2, a";}
        case 0xd8: {return "set 3, b";} case 0xd9: {return "set 3, c";} case 0xda: {return "set 3, d";} case 0xdb: {return "set 3, e";}
        case 0xdc: {return "set 3, h";} case 0xdd: {return "set 3, l";} case 0xde: {return "set 3, (hl)";} case 0xdf: {return "set 3, a";}
        case 0xe0: {return "set 4, b";} case 0xe1: {return "set 4, c";} case 0xe2: {return "set 4, d";} case 0xe3: {return "set 4, e";}
        case 0xe4: {return "set 4, h";} case 0xe5: {return "set 4, l";} case 0xe6: {return "set 4, (hl)";} case 0xe7: {return "set 4, a";}
        case 0xe8: {return "set 5, b";} case 0xe9: {return "set 5, c";} case 0xea: {return "set 5, d";} case 0xeb: {return "set 5, e";}
        case 0xec: {return "set 5, h";} case 0xed: {return "set 5, l";} case 0xee: {return "set 5, (hl)";} case 0xef: {return "set 5, a";}
        case 0xf0: {return "set 6, b";} case 0xf1: {return "set 6, c";} case 0xf2: {return "set 6, d";} case 0xf3: {return "set 6, e";}
        case 0xf4: {return "set 6, h";} case 0xf5: {return "set 6, l";} case 0xf6: {return "set 6, (hl)";} case 0xf7: {return "set 6, a";}
        case 0xf8: {return "set 7, b";} case 0xf9: {return "set 7, c";} case 0xfa: {return "set 7, d";} case 0xfb: {return "set 7, e";}
        case 0xfc: {return "set 7, h";} case 0xfd: {return "set 7, l";} case 0xfe: {return "set 7, (hl)";} case 0xff: {return "set 7, a";}
    }

    return "nothing";
}

