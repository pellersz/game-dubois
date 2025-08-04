#include "cpu.h"
#include "scheduler.h"
#include "types.h"

/////////////////////////////////////////////////////////////////
// Helper instructions
/////////////////////////////////////////////////////////////////


// load 
void Cpu::opLd(byte& dest, byte src) { dest = src; } 

void Cpu::opLd(word& dest, word src) { dest = src; }

void Cpu::opLd(byte& dest_lo, byte& dest_hi, word src) 
{ 
    dest_hi = src >> 8;
    dest_lo = src;
}

// arithmetic

void Cpu::opAdd(byte val) 
{
    byte tmp = A;
    A += val;
    setCF(A < tmp); 
    setZF(!A); 
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

void Cpu::opAdd(word val) 
{
    byte tmp = getHL();
    byte new_val = tmp + val;
    setHL(new_val); 
    setCF(new_val < tmp); 
    setHF((A & 0b111111111111u) < (tmp & 0b111111111111u));
    setNF(false);
}

void Cpu::opAddSP(offs val) {
    byte tmp = SP;
    SP = tmp + val;
    setCF(SP < tmp); 
    setHF((A & 0b111111111111u) < (tmp & 0b111111111111u));
    F &= HF_MASK & CF_MASK;
}

void Cpu::opAdc(byte val) 
{
    byte tmp = A;
    A += val + getCF(); 
    setCF((A < tmp) || (A == tmp && getCF()));
    setZF(!A);
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

void Cpu::opSub(byte val) 
{
    byte tmp = A;
    A -= val; 
    setCF(A > tmp); 
    setZF(!A); 
    setHF((A & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}

void Cpu::opSbc(byte val) 
{
    byte tmp = A;
    A -= val + getCF(); 
    setCF(tmp < val + getCF()); 
    setZF(!A); 
    setHF((A & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}


void Cpu::opCp(byte val) 
{
    byte tmp = A - val;
    setCF(A < tmp); 
    setZF(!tmp); 
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(true);
}

void Cpu::opInc(byte& dest) {
    byte tmp = dest++;
    setZF(!dest); 
    setHF((dest & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

void Cpu::opInc(word& dest) { ++dest; }

void Cpu::opInc(byte& lo, byte& hi) { hi += !(lo += 1); }

void Cpu::opDec(byte& dest) {
    byte tmp = dest--;
    dest -= 1; 
    setZF(!dest); 
    setHF((dest & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}

void Cpu::opDec(word& dest) { --dest; }

void Cpu::opDec(byte& lo, byte& hi) { hi -= ((lo -= 1) == (byte)(-1)); }

// logic

void Cpu::opAnd(byte val) 
{
    A &= val; 
    F &= NULLF_MASK;
    setHF(true);
    setZF(!A); 
}

void Cpu::opOr(byte val) 
{
    A &= val; 
    F &= NULLF_MASK;
    setZF(!A); 
}

void Cpu::opXor(byte val)
{
    A ^= val; 
    F &= NULLF_MASK;
    setZF(!A); 
}

void Cpu::opCpl()
{
    A = ~A;
    F |= NF_MASK | HF_MASK;
}

//bit
void Cpu::opBit(byte shift, byte val) 
{
    setZF(val & 1u << shift);
    setNF(false);
    setHF(false);
}

void Cpu::opRes(byte shift, byte& dest) { dest &= ~(1u << shift); } 

void Cpu::opSet(byte shift, byte& dest) { dest |= (1u << shift); }

// bitshift

void Cpu::opRl(byte& dest) 
{
    bool new_CF = dest & 0b10000000u;
    dest = (dest << 1) + getCF();
    setCF(new_CF);
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opRla()
{
    bool new_CF = A & 0b10000000u;
    A = (A << 1) + getCF();
    setCF(new_CF);
    setZF(!A);
    F &= CF_MASK;
}

void Cpu::opRlc(byte& dest)
{
    bool last_set = dest & 0b10000000u;
    dest = (dest << 1) + last_set;
    setCF(last_set);
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opRlca()
{
    bool last_set = A & 0b10000000u;
    A = (A << 1) + last_set;
    setCF(last_set);
    setZF(!A);
    F &= CF_MASK;
}

void Cpu::opRr(byte& dest)
{
    bool new_CF = dest & 0b00000001u;
    dest = (dest >> 1) + getCF() * 0b10000000u;
    setCF(new_CF);
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opRra()
{
    bool new_CF = A & 0b00000001u;
    A = (A >> 1) + getCF() * 0b10000000u;
    setCF(new_CF);
    setZF(!A);
    F &= CF_MASK;
}

void Cpu::opRrc(byte& dest)
{
    bool last_set = dest & 0b00000001u;
    dest = (dest >> 1) + last_set * 0b10000000u;
    setCF(last_set);
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opRrca()
{
    bool new_CF = A & 0b00000001u;
    A = (A >> 1) + getCF() * 0b10000000u;
    setCF(new_CF);
    setZF(!A);
    F &= CF_MASK;
}

void Cpu::opSla(byte& dest)
{
    setCF(dest & 0b10000000u);
    dest <<= 1;
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opSra(byte& dest)
{
    setCF(dest & 0b00000001u);
    dest = (dest & 0b10000000u) + (dest >> 1);
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opSrl(byte& dest)
{
    setCF(dest & 0b00000001u);
    dest >>= 1;
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opSwap(byte& dest)
{
    dest = (dest << 4) + (dest >> 4);
    setZF(!dest);
    F &= ZF_MASK;
}

// jumps and subroutine

void Cpu::opCall(word addr)
{
    opPush(PC);   
    PC = addr;
}

void Cpu::opCall(bool jump, word addr)
{
    if (jump) {
        opCall(addr);
        scheduler.push(6, CPU_EXEC);
        return;
    }
    scheduler.push(3, CPU_EXEC);
    programCounterStep(3);
}

void Cpu::opJp(word addr) { PC = addr; }

void Cpu::opJp(bool jump, word addr)
{
    if (jump) {
        opJp(addr);
        scheduler.push(4, CPU_EXEC);
        return;
    }
    scheduler.push(3, CPU_EXEC);
    programCounterStep(3);
}

void Cpu::opJr(offs addr) { PC += addr; }

void Cpu::opJr(bool jump, offs addr)
{
    if (jump) {
        opJr(addr);
        scheduler.push(3, CPU_EXEC);
        return;
    }
    scheduler.push(2, CPU_EXEC);
    programCounterStep(2);
}

void Cpu::opRet() { opPop(PC); }

void Cpu::opRet(bool jump)
{
    if (jump) {
        opRet();
        scheduler.push(5, CPU_EXEC);
        return;
    }
    scheduler.push(2, CPU_EXEC);
    programCounterStep(1); 
}

void Cpu::opReti()
{
    opRet();
    opEi();
}

void Cpu::opRst(u8 addr) { opCall(addr); }

// carry flag

void Cpu::opCcf()
{
    setCF(~getCF());
    setNF(false);
    setHF(false);
}

void Cpu::opScf()
{
    setCF(true);
    F &= ZF_MASK | CF_MASK;
}

// stack manipulation

void Cpu::opAdd(offs val)
{
    word tmp = SP;
    SP += val;
    F &= HF_MASK | CF_MASK;
    setCF((SP & 0b11111111) < (tmp & 0b11111111)); 
    setHF((SP & 0b1111u) < (tmp & 0b1111u));
}

void Cpu::opPop(word& reg)
{
    reg = memory(SP);
    stack2StepBack();
}

void Cpu::opPop(byte& reg_lo, byte& reg_hi)
{
    reg_lo = memory[SP];
    reg_hi = memory[SP + 1];
    stack2StepBack();
}

void Cpu::opPush(word reg)
{
    memory.writeWord(SP, reg);
    stack2Step();
}

void Cpu::opDi() { IME = false; }

void Cpu::opEi() { IME = true; }

void Cpu::opHalt() { halted = true; }

// misc  

void Cpu::opDaa()
{
    byte tmp = A;
    setHF(false);

    if (getNF()) 
    {
        if (getHF())
            A -= 0x6;
        if (getCF())
            A -= 0x60;
        setCF(A > tmp);
        setZF(!A);
        return;
    }
    if (getHF() || ((A & 0b1111) > 0x9))
        A += 0x6;
    if (getCF() || (A > 0x9F))
        A += 0x60;
    setCF(A < tmp);
}

void Cpu::opNop() {}

void Cpu::opStop() {/*most likely not gonna use*/}


/////////////////////////////////////////////////////////////////
// Regular (8 bit) operations
/////////////////////////////////////////////////////////////////

// some instructions have variable cycles, so they might insert the next instruction, instead of the main exexecute function, also progress the PC further
void Cpu::op_0x00() { opNop(); } 

void Cpu::op_0x01() { opLd(C, B, memory(PC + 1)); } 

void Cpu::op_0x02() { opLd(memory[getBC()], A); } 

void Cpu::op_0x03() { opInc(C, B); } 

void Cpu::op_0x04() { opInc(B); } 

void Cpu::op_0x05() { opDec(B); } 

void Cpu::op_0x06() { opLd(B, memory(PC + 1)); } 

void Cpu::op_0x07() { opRlca(); } 

void Cpu::op_0x08() 
{ 
    unsigned short offs = memory(PC + 1);
    opLd(memory[offs], memory[offs + 1], SP); 
} 

void Cpu::op_0x09() { opAdd(getBC()); } 

void Cpu::op_0x0a() { opLd(A, memory[getBC()]); } 

void Cpu::op_0x0b() { opDec(C, B); } 

void Cpu::op_0x0c() { opInc(C); } 

void Cpu::op_0x0d() { opDec(C); } 

void Cpu::op_0x0e() { opLd(C, memory[PC + 1]); } 

void Cpu::op_0x0f() { opRrca(); } 

void Cpu::op_0x10() { opStop(); } 

void Cpu::op_0x11() { opLd(E, D, memory(PC + 1)); } 

void Cpu::op_0x12() { opLd(memory[getDE()], A); } 

void Cpu::op_0x13() { opInc(E, D); } 

void Cpu::op_0x14() { opInc(D); } 

void Cpu::op_0x15() { opDec(D); } 

void Cpu::op_0x16() { opLd(D, memory[PC + 1]); } 

void Cpu::op_0x17() { opRla(); } 

void Cpu::op_0x18() { opJr(memory[PC + 1]); } 

void Cpu::op_0x19() { opAdd(getDE()); } 

void Cpu::op_0x1a() { opLd(A, memory[PC + 1]); } 

void Cpu::op_0x1b() { opDec(E, D); } 

void Cpu::op_0x1c() { opInc(E); } 

void Cpu::op_0x1d() { opDec(E); } 

void Cpu::op_0x1e() { opLd(E, memory[PC + 1]); } 

void Cpu::op_0x1f() { opRra(); } 

void Cpu::op_0x20() { opJr(!getZF(), memory[PC + 1]); } 

void Cpu::op_0x21() { opLd(L, H, memory(memory(PC + 1))); } 

void Cpu::op_0x22() { 
    opLd(memory[getHL()], A);
    opInc(L, H);
} 

void Cpu::op_0x23() { opInc(L, H); } 

void Cpu::op_0x24() { opInc(H); } 

void Cpu::op_0x25() { opDec(H); } 

void Cpu::op_0x26() { opLd(H, memory[PC + 1]); } 

void Cpu::op_0x27() { opDaa(); } 

void Cpu::op_0x28() { opJr(getZF(), memory[PC + 1]); }

void Cpu::op_0x29() { opAdd(getHL()); } 

void Cpu::op_0x2a() 
{ 
    opLd(A, getHL()); 
    opInc(L, H);
} 

void Cpu::op_0x2b() { opDec(L, H); } 

void Cpu::op_0x2c() { opInc(L); } 

void Cpu::op_0x2d() { opDec(L); } 

void Cpu::op_0x2e() { opLd(L, memory[PC + 1]); } 

void Cpu::op_0x2f() { opCpl(); } 

void Cpu::op_0x30() { opJr(!getCF(), memory[PC + 1]); } 

void Cpu::op_0x31() { opLd(SP, memory(PC + 1)); } 

void Cpu::op_0x32() { opLd(memory[getHL()], A); } 

void Cpu::op_0x33() { opInc(SP); } 

void Cpu::op_0x34() 
{ 
    unsigned short tmp = getHL();
    opInc(memory[tmp], memory[tmp + 1]);
    opInc(L, H);
} 

void Cpu::op_0x35() 
{ 
    unsigned short tmp = getHL();
    opInc(memory[tmp], memory[tmp + 1]);
    opDec(L, H);
} 

void Cpu::op_0x36() { opLd(memory[getHL()], memory[PC + 1]); } 

void Cpu::op_0x37() { opScf(); } 

void Cpu::op_0x38() { opJr(getCF(), memory[PC + 1]); } 

void Cpu::op_0x39() { opAdd(SP); } 

void Cpu::op_0x3a() 
{
    opLd(A, memory[getHL()]);
    opInc(L, H);
} 

void Cpu::op_0x3b() { opDec(SP); } 

void Cpu::op_0x3c() { opInc(A); } 

void Cpu::op_0x3d() { opDec(A); } 

void Cpu::op_0x3e() { opLd(A, memory[PC + 1]); } 

void Cpu::op_0x3f() { opCcf(); } 

void Cpu::op_0x40() { opLd(B, B); } 

void Cpu::op_0x41() { opLd(B, C); } 

void Cpu::op_0x42() { opLd(B, D); } 

void Cpu::op_0x43() { opLd(B, E); } 

void Cpu::op_0x44() { opLd(B, H); } 

void Cpu::op_0x45() { opLd(B, L); } 

void Cpu::op_0x46() { opLd(B, memory[getHL()]);  } 

void Cpu::op_0x47() { opLd(B, A); } 

void Cpu::op_0x48() { opLd(C, B); } 

void Cpu::op_0x49() { opLd(C, C); } 

void Cpu::op_0x4a() { opLd(C, D); } 

void Cpu::op_0x4b() { opLd(C, E); } 

void Cpu::op_0x4c() { opLd(C, H); } 

void Cpu::op_0x4d() { opLd(C, L); } 

void Cpu::op_0x4e() { opLd(C, memory[getHL()]); } 

void Cpu::op_0x4f() { opLd(C, A); } 

void Cpu::op_0x50() { opLd(D, B); } 

void Cpu::op_0x51() { opLd(D, C); } 

void Cpu::op_0x52() { opLd(D, D); } 

void Cpu::op_0x53() { opLd(D, E); } 

void Cpu::op_0x54() { opLd(D, H); } 

void Cpu::op_0x55() { opLd(D, L); } 

void Cpu::op_0x56() { opLd(D, memory[getHL()]); } 

void Cpu::op_0x57() { opLd(D, A); } 

void Cpu::op_0x58() { opLd(E, B); } 

void Cpu::op_0x59() { opLd(E, C); } 

void Cpu::op_0x5a() { opLd(E, D); } 

void Cpu::op_0x5b() { opLd(E, E); } 

void Cpu::op_0x5c() { opLd(E, H); } 

void Cpu::op_0x5d() { opLd(E, L); } 

void Cpu::op_0x5e() { opLd(E, memory[getHL()]); } 

void Cpu::op_0x5f() { opLd(E, A); } 

void Cpu::op_0x60() { opLd(H, B); } 

void Cpu::op_0x61() { opLd(H, C); } 

void Cpu::op_0x62() { opLd(H, D); } 

void Cpu::op_0x63() { opLd(H, E); } 

void Cpu::op_0x64() { opLd(H, H); } 

void Cpu::op_0x65() { opLd(H, L); } 

void Cpu::op_0x66() { opLd(H, memory[getHF()]); } 

void Cpu::op_0x67() { opLd(H, A); } 

void Cpu::op_0x68() { opLd(L, B); } 

void Cpu::op_0x69() { opLd(L, C); } 

void Cpu::op_0x6a() { opLd(L, D); } 

void Cpu::op_0x6b() { opLd(L, E); } 

void Cpu::op_0x6c() { opLd(L, H); } 

void Cpu::op_0x6d() { opLd(L, L); } 

void Cpu::op_0x6e() { opLd(L, memory[getHL()]); } 

void Cpu::op_0x6f() { opLd(L, A); } 

void Cpu::op_0x70() { opLd(memory[getHL()], B); } 

void Cpu::op_0x71() { opLd(memory[getHL()], C); } 

void Cpu::op_0x72() { opLd(memory[getHL()], D); } 

void Cpu::op_0x73() { opLd(memory[getHL()], E); } 

void Cpu::op_0x74() { opLd(memory[getHL()], H); } 

void Cpu::op_0x75() { opLd(memory[getHL()], L); } 

void Cpu::op_0x76() { opHalt(); } 

void Cpu::op_0x77() { opLd(memory[getHL()], A); } 

void Cpu::op_0x78() { opLd(A, B); } 

void Cpu::op_0x79() { opLd(A, C); } 

void Cpu::op_0x7a() { opLd(A, D); } 

void Cpu::op_0x7b() { opLd(A, E); } 

void Cpu::op_0x7c() { opLd(A, H); } 

void Cpu::op_0x7d() { opLd(A, L); } 

void Cpu::op_0x7e() { opLd(A, memory[getHL()]); } 

void Cpu::op_0x7f() { opLd(A, A); } 

void Cpu::op_0x80() { opAdd(B); } 

void Cpu::op_0x81() { opAdd(C); } 

void Cpu::op_0x82() { opAdd(D); } 

void Cpu::op_0x83() { opAdd(E); } 

void Cpu::op_0x84() { opAdd(H); } 

void Cpu::op_0x85() { opAdd(L); } 

void Cpu::op_0x86() { opAdd(memory[getHL()]); } 

void Cpu::op_0x87() { opAdd(A); } 

void Cpu::op_0x88() { opAdc(B); } 

void Cpu::op_0x89() { opAdc(C); } 

void Cpu::op_0x8a() { opAdc(D); } 

void Cpu::op_0x8b() { opAdc(E); } 

void Cpu::op_0x8c() { opAdc(H); } 

void Cpu::op_0x8d() { opAdc(L); } 

void Cpu::op_0x8e() { opAdc(memory[getHL()]); } 

void Cpu::op_0x8f() { opAdc(A); } 

void Cpu::op_0x90() { opSub(B); } 

void Cpu::op_0x91() { opSub(C); } 

void Cpu::op_0x92() { opSub(D); } 

void Cpu::op_0x93() { opSub(E); } 

void Cpu::op_0x94() { opSub(H); } 

void Cpu::op_0x95() { opSub(L); } 

void Cpu::op_0x96() { opSub(memory[getHL()]); } 

void Cpu::op_0x97() { opSub(A); } 

void Cpu::op_0x98() { opSbc(B); } 

void Cpu::op_0x99() { opSbc(C); } 

void Cpu::op_0x9a() { opSbc(D); } 

void Cpu::op_0x9b() { opSbc(E); } 

void Cpu::op_0x9c() { opSbc(H); } 

void Cpu::op_0x9d() { opSbc(L); } 

void Cpu::op_0x9e() { opSbc(memory[getHL()]); } 

void Cpu::op_0x9f() { opSbc(A); } 

void Cpu::op_0xa0() { opAnd(B); } 

void Cpu::op_0xa1() { opAnd(C); } 

void Cpu::op_0xa2() { opAnd(D); } 

void Cpu::op_0xa3() { opAnd(E); } 

void Cpu::op_0xa4() { opAnd(H); } 

void Cpu::op_0xa5() { opAnd(L); } 

void Cpu::op_0xa6() { opAnd(memory[getHL()]); } 

void Cpu::op_0xa7() { opAnd(A); } 

void Cpu::op_0xa8() { opXor(B); } 

void Cpu::op_0xa9() { opXor(C); } 

void Cpu::op_0xaa() { opXor(D); } 

void Cpu::op_0xab() { opXor(E); } 

void Cpu::op_0xac() { opXor(H); } 

void Cpu::op_0xad() { opXor(L); } 

void Cpu::op_0xae() { opXor(memory[getHL()]); } 

void Cpu::op_0xaf() { opXor(A); } 

void Cpu::op_0xb0() { opOr(B); } 

void Cpu::op_0xb1() { opOr(C); } 

void Cpu::op_0xb2() { opOr(D); } 

void Cpu::op_0xb3() { opOr(E); } 

void Cpu::op_0xb4() { opOr(H); } 

void Cpu::op_0xb5() { opOr(L); } 

void Cpu::op_0xb6() { opOr(memory[getHL()]); } 

void Cpu::op_0xb7() { opOr(A); } 

void Cpu::op_0xb8() { opCp(B); } 

void Cpu::op_0xb9() { opCp(C); } 

void Cpu::op_0xba() { opCp(D); } 

void Cpu::op_0xbb() { opCp(E); } 

void Cpu::op_0xbc() { opCp(H); } 

void Cpu::op_0xbd() { opCp(L); } 

void Cpu::op_0xbe() { opCp(memory[getHL()]); } 

void Cpu::op_0xbf() { opCp(A); } 

void Cpu::op_0xc0() { opRet(!getZF()); } 

void Cpu::op_0xc1() { opPop(C, B); } 

void Cpu::op_0xc2() { opJp(!getZF(), memory(PC + 1)); } 

void Cpu::op_0xc3() { opJp(memory(PC + 1)); } 

void Cpu::op_0xc4() { opCall(!getZF(), memory[PC + 1]); } 

void Cpu::op_0xc5() { opPush(getBC()); } 

void Cpu::op_0xc6() { opAdd(memory[PC + 1]); } 

void Cpu::op_0xc7() { opRst(8 * 0); } 

void Cpu::op_0xc8() { opRet(getZF()); } 

void Cpu::op_0xc9() { opRet(); } 

void Cpu::op_0xca() { opJp(getZF(), memory(PC + 1)); } 

void Cpu::op_0xcb() { executeBC(memory[PC + 1]); } 

void Cpu::op_0xcc() { opCall(getZF(), memory(PC + 1)); } 

void Cpu::op_0xcd() { opCall(memory(PC + 1)); } 

void Cpu::op_0xce() { opAdc(memory[PC + 1]); } 

void Cpu::op_0xcf() { opRst(8 * 1); } 

void Cpu::op_0xd0() { opRet(!getCF()); } 

void Cpu::op_0xd1() { opPop(E, D); } 

void Cpu::op_0xd2() { opJp(!getCF(), memory(PC + 1)); } 

void Cpu::op_0xd3() { opNop(); }   // I'm gonna be generous, and not crash the console when it finds an unknown op code 

void Cpu::op_0xd4() { opCall(!getCF(), memory(PC + 1)); } 

void Cpu::op_0xd5() { opPush(getDE()); } 

void Cpu::op_0xd6() { opSub(memory[PC + 1]); } 

void Cpu::op_0xd7() { opRst(8 * 2); } 

void Cpu::op_0xd8() { opRet(getCF()); } 

void Cpu::op_0xd9() { opReti(); } 

void Cpu::op_0xda() { opJp(getCF(), memory(PC + 1)); } 

void Cpu::op_0xdb() { opNop(); } 

void Cpu::op_0xdc() { opCall(getCF(), memory(PC + 1)); } 

void Cpu::op_0xdd() { opNop(); } 

void Cpu::op_0xde() { opSbc(memory[PC + 1]); } 

void Cpu::op_0xdf() { opRst(8 * 3); } 

void Cpu::op_0xe0() { opLd(memory[0x00ff + memory[PC + 1]], A); } 

void Cpu::op_0xe1() { opPop(L, H); } 

void Cpu::op_0xe2() { opLd(memory[0x00ff + C], A); } 

void Cpu::op_0xe3() { opNop(); } 

void Cpu::op_0xe4() { opNop(); } 

void Cpu::op_0xe5() { opPush(getHL()); } 

void Cpu::op_0xe6() { opAnd(memory[PC + 1]); } 

void Cpu::op_0xe7() { opRst(8 * 4); } 

void Cpu::op_0xe8() { opAddSP(memory[PC + 1]); } 

void Cpu::op_0xe9() { opJp(getHL()); } 

void Cpu::op_0xea() { opLd(memory[memory(PC + 1)], A); } 

void Cpu::op_0xeb() { opNop(); } 

void Cpu::op_0xec() { opNop(); } 

void Cpu::op_0xed() { opNop(); } 

void Cpu::op_0xee() { opXor(memory[PC + 1]); } 

void Cpu::op_0xef() { opRst(8 * 5); } 

void Cpu::op_0xf0() { opLd(A, memory[0xff00 + memory[PC + 1]]);} 

void Cpu::op_0xf1() { opPop(F, A); } 

void Cpu::op_0xf2() { opLd(A, memory[0xff00 + C]); } 

void Cpu::op_0xf3() { opDi(); } 

void Cpu::op_0xf4() { opNop(); } 

void Cpu::op_0xf5() { opPush(getAF()); } 

void Cpu::op_0xf6() { opOr(memory[PC + 1]); } 

void Cpu::op_0xf7() { opRst(8 * 6); } 

void Cpu::op_0xf8() { opLd(L, H, SP + memory[PC + 1]); } 

void Cpu::op_0xf9() { opLd(SP, getHL()); } 

void Cpu::op_0xfa() { opLd(A, memory[memory(PC + 1)]); } 

void Cpu::op_0xfb() { opEi(); } 

void Cpu::op_0xfc() { opNop(); } 

void Cpu::op_0xfd() { opNop(); } 

void Cpu::op_0xfe() { opCp(memory[PC + 1]); } 

void Cpu::op_0xff() { opRst(8 * 7); } 


/////////////////////////////////////////////////////////////////
// bc (16 bit) operations
/////////////////////////////////////////////////////////////////


void Cpu::opCb_0x00() { opRlc(B); } 

void Cpu::opCb_0x01() { opRlc(C); } 

void Cpu::opCb_0x02() { opRlc(D); } 

void Cpu::opCb_0x03() { opRlc(E); } 

void Cpu::opCb_0x04() { opRlc(H); } 

void Cpu::opCb_0x05() { opRlc(L); } 

void Cpu::opCb_0x06() { opRlc(memory[getHL()]); } 

void Cpu::opCb_0x07() { opRlc(A); } 

void Cpu::opCb_0x08() { opRrc(B); } 

void Cpu::opCb_0x09() { opRrc(C); } 

void Cpu::opCb_0x0a() { opRrc(D); } 

void Cpu::opCb_0x0b() { opRrc(E); } 

void Cpu::opCb_0x0c() { opRrc(H); } 

void Cpu::opCb_0x0d() { opRrc(L); } 

void Cpu::opCb_0x0e() { opRrc(memory[getHL()]); } 

void Cpu::opCb_0x0f() { opRrc(A); } 

void Cpu::opCb_0x10() { opRl(B); } 

void Cpu::opCb_0x11() { opRl(C); } 

void Cpu::opCb_0x12() { opRl(D); } 

void Cpu::opCb_0x13() { opRl(E); } 

void Cpu::opCb_0x14() { opRl(H); } 

void Cpu::opCb_0x15() { opRl(L); } 

void Cpu::opCb_0x16() { opRl(memory[getHL()]); } 

void Cpu::opCb_0x17() { opRl(A); } 

void Cpu::opCb_0x18() { opRr(B); } 

void Cpu::opCb_0x19() { opRr(C); } 

void Cpu::opCb_0x1a() { opRr(D); } 

void Cpu::opCb_0x1b() { opRr(E); } 

void Cpu::opCb_0x1c() { opRr(H); } 

void Cpu::opCb_0x1d() { opRr(L); } 

void Cpu::opCb_0x1e() { opRr(memory[getHL()]); } 

void Cpu::opCb_0x1f() { opRr(A); } 

void Cpu::opCb_0x20() { opSla(B); } 

void Cpu::opCb_0x21() { opSla(C); } 

void Cpu::opCb_0x22() { opSla(D); } 

void Cpu::opCb_0x23() { opSla(E); } 

void Cpu::opCb_0x24() { opSla(H); } 

void Cpu::opCb_0x25() { opSla(L); } 

void Cpu::opCb_0x26() { opSla(memory[getHL()]); } 

void Cpu::opCb_0x27() { opSla(A); } 

void Cpu::opCb_0x28() { opSra(B); } 

void Cpu::opCb_0x29() { opSra(C); } 

void Cpu::opCb_0x2a() { opSra(D); } 

void Cpu::opCb_0x2b() { opSra(E); } 

void Cpu::opCb_0x2c() { opSra(H); } 

void Cpu::opCb_0x2d() { opSra(L); } 

void Cpu::opCb_0x2e() { opSra(memory[getHL()]); } 

void Cpu::opCb_0x2f() { opSra(A); } 

void Cpu::opCb_0x30() { opSwap(B); } 

void Cpu::opCb_0x31() { opSwap(C); } 

void Cpu::opCb_0x32() { opSwap(D); } 

void Cpu::opCb_0x33() { opSwap(E); } 

void Cpu::opCb_0x34() { opSwap(H); } 

void Cpu::opCb_0x35() { opSwap(L); } 

void Cpu::opCb_0x36() { opSwap(memory[getHL()]); } 

void Cpu::opCb_0x37() { opSwap(A); } 

void Cpu::opCb_0x38() { opSrl(B); } 

void Cpu::opCb_0x39() { opSrl(C); } 

void Cpu::opCb_0x3a() { opSrl(D); } 

void Cpu::opCb_0x3b() { opSrl(E); } 

void Cpu::opCb_0x3c() { opSrl(H); } 

void Cpu::opCb_0x3d() { opSrl(L); } 

void Cpu::opCb_0x3e() { opSrl(memory[getHL()]); } 

void Cpu::opCb_0x3f() { opSrl(A); } 

void Cpu::opCb_0x40() { opBit(0, B); } 

void Cpu::opCb_0x41() { opBit(0, C); } 

void Cpu::opCb_0x42() { opBit(0, D); } 

void Cpu::opCb_0x43() { opBit(0, E); } 

void Cpu::opCb_0x44() { opBit(0, H); } 

void Cpu::opCb_0x45() { opBit(0, L); } 

void Cpu::opCb_0x46() { opBit(0, memory[getHL()]); } 

void Cpu::opCb_0x47() { opBit(0, A); } 

void Cpu::opCb_0x48() { opBit(1, B); } 

void Cpu::opCb_0x49() { opBit(1, C); } 

void Cpu::opCb_0x4a() { opBit(1, D); } 

void Cpu::opCb_0x4b() { opBit(1, E); } 

void Cpu::opCb_0x4c() { opBit(1, H); } 

void Cpu::opCb_0x4d() { opBit(1, L); } 

void Cpu::opCb_0x4e() { opBit(1, memory[getHL()]); } 

void Cpu::opCb_0x4f() { opBit(1, A); } 

void Cpu::opCb_0x50() { opBit(2, B); } 

void Cpu::opCb_0x51() { opBit(2, C); } 

void Cpu::opCb_0x52() { opBit(2, D); } 

void Cpu::opCb_0x53() { opBit(2, E); } 

void Cpu::opCb_0x54() { opBit(2, H); } 

void Cpu::opCb_0x55() { opBit(2, L); } 

void Cpu::opCb_0x56() { opBit(2, memory[getHF()]); } 

void Cpu::opCb_0x57() { opBit(2, A); } 

void Cpu::opCb_0x58() { opBit(3, B); } 

void Cpu::opCb_0x59() { opBit(3, C); } 

void Cpu::opCb_0x5a() { opBit(3, D); } 

void Cpu::opCb_0x5b() { opBit(3, E); } 

void Cpu::opCb_0x5c() { opBit(3, H); } 

void Cpu::opCb_0x5d() { opBit(3, L); } 

void Cpu::opCb_0x5e() { opBit(3, memory[getHL()]); } 

void Cpu::opCb_0x5f() { opBit(3, A); } 

void Cpu::opCb_0x60() { opBit(4, B); } 

void Cpu::opCb_0x61() { opBit(4, C); } 

void Cpu::opCb_0x62() { opBit(4, D); } 

void Cpu::opCb_0x63() { opBit(4, E); } 

void Cpu::opCb_0x64() { opBit(4, H); } 

void Cpu::opCb_0x65() { opBit(4, L); } 

void Cpu::opCb_0x66() { opBit(4, memory[getHF()]); } 

void Cpu::opCb_0x67() { opBit(4, A); } 

void Cpu::opCb_0x68() { opBit(5, B); } 

void Cpu::opCb_0x69() { opBit(5, C); } 

void Cpu::opCb_0x6a() { opBit(5, D); } 

void Cpu::opCb_0x6b() { opBit(5, E); } 

void Cpu::opCb_0x6c() { opBit(5, H); } 

void Cpu::opCb_0x6d() { opBit(5, L); } 

void Cpu::opCb_0x6e() { opBit(5, memory[getHL()]); } 

void Cpu::opCb_0x6f() { opBit(5, A); } 

void Cpu::opCb_0x70() { opBit(6, B); } 

void Cpu::opCb_0x71() { opBit(6, C); } 

void Cpu::opCb_0x72() { opBit(6, D); } 

void Cpu::opCb_0x73() { opBit(6, E); } 

void Cpu::opCb_0x74() { opBit(6, H); } 

void Cpu::opCb_0x75() { opBit(6, L); } 

void Cpu::opCb_0x76() { opBit(6, memory[getHL()]); } 

void Cpu::opCb_0x77() { opBit(6, A); } 

void Cpu::opCb_0x78() { opBit(7, B); } 

void Cpu::opCb_0x79() { opBit(7, C); } 

void Cpu::opCb_0x7a() { opBit(7, D); } 

void Cpu::opCb_0x7b() { opBit(7, E); } 

void Cpu::opCb_0x7c() { opBit(7, H); } 

void Cpu::opCb_0x7d() { opBit(7, L); } 

void Cpu::opCb_0x7e() { opBit(7, memory[getHL()]); } 

void Cpu::opCb_0x7f() { opBit(7, A); } 

void Cpu::opCb_0x80() { opRes(0, B); } 

void Cpu::opCb_0x81() { opRes(0, C); } 

void Cpu::opCb_0x82() { opRes(0, D); } 

void Cpu::opCb_0x83() { opRes(0, E); } 

void Cpu::opCb_0x84() { opRes(0, H); } 

void Cpu::opCb_0x85() { opRes(0, L); } 

void Cpu::opCb_0x86() { opRes(0, memory[getHL()]); } 

void Cpu::opCb_0x87() { opRes(0, A); } 

void Cpu::opCb_0x88() { opRes(1, B); } 

void Cpu::opCb_0x89() { opRes(1, C); } 

void Cpu::opCb_0x8a() { opRes(1, D); } 

void Cpu::opCb_0x8b() { opRes(1, E); } 

void Cpu::opCb_0x8c() { opRes(1, H); } 

void Cpu::opCb_0x8d() { opRes(1, L); } 

void Cpu::opCb_0x8e() { opRes(1, memory[getHL()]);} 

void Cpu::opCb_0x8f() { opRes(1, A); } 

void Cpu::opCb_0x90() { opRes(2, B); } 

void Cpu::opCb_0x91() { opRes(2, C); } 

void Cpu::opCb_0x92() { opRes(2, D); } 

void Cpu::opCb_0x93() { opRes(2, E); } 

void Cpu::opCb_0x94() { opRes(2, H); } 

void Cpu::opCb_0x95() { opRes(2, L); } 

void Cpu::opCb_0x96() { opRes(2, memory[getHL()]); } 

void Cpu::opCb_0x97() { opRes(2, A); } 

void Cpu::opCb_0x98() { opRes(3, B); } 

void Cpu::opCb_0x99() { opRes(3, C); } 

void Cpu::opCb_0x9a() { opRes(3, D); } 

void Cpu::opCb_0x9b() { opRes(3, E); } 

void Cpu::opCb_0x9c() { opRes(3, H); } 

void Cpu::opCb_0x9d() { opRes(3, L); } 

void Cpu::opCb_0x9e() { opRes(3, memory[getHL()]); } 

void Cpu::opCb_0x9f() { opRes(3, A); } 

void Cpu::opCb_0xa0() { opRes(4, B); } 

void Cpu::opCb_0xa1() { opRes(4, C);} 

void Cpu::opCb_0xa2() { opRes(4, D); } 

void Cpu::opCb_0xa3() { opRes(4, E); } 

void Cpu::opCb_0xa4() { opRes(4, H); } 

void Cpu::opCb_0xa5() { opRes(4, L); } 

void Cpu::opCb_0xa6() { opRes(4, memory[getHL()]); } 

void Cpu::opCb_0xa7() { opRes(4, A); } 

void Cpu::opCb_0xa8() { opRes(5, B); } 

void Cpu::opCb_0xa9() { opRes(5, C); } 

void Cpu::opCb_0xaa() { opRes(5, D); } 

void Cpu::opCb_0xab() { opRes(5, E); } 

void Cpu::opCb_0xac() { opRes(5, H); } 

void Cpu::opCb_0xad() { opRes(5, L); } 

void Cpu::opCb_0xae() { opRes(5, memory[getHL()]); } 

void Cpu::opCb_0xaf() { opRes(5, A); } 

void Cpu::opCb_0xb0() { opRes(6, B); } 

void Cpu::opCb_0xb1() { opRes(6, C);  } 

void Cpu::opCb_0xb2() { opRes(6, D); } 

void Cpu::opCb_0xb3() { opRes(6, E);  } 

void Cpu::opCb_0xb4() { opRes(6, H); } 

void Cpu::opCb_0xb5() { opRes(6, L); } 

void Cpu::opCb_0xb6() { opRes(6, memory[getHL()]); } 

void Cpu::opCb_0xb7() { opRes(6, A); } 

void Cpu::opCb_0xb8() { opRes(7, B); } 

void Cpu::opCb_0xb9() { opRes(7, C); } 

void Cpu::opCb_0xba() { opRes(7, D); } 

void Cpu::opCb_0xbb() { opRes(7, E); } 

void Cpu::opCb_0xbc() { opRes(7, H); } 

void Cpu::opCb_0xbd() { opRes(7, L); } 

void Cpu::opCb_0xbe() { opRes(7, memory[getHL()]); } 

void Cpu::opCb_0xbf() { opRes(7, A); } 

void Cpu::opCb_0xc0() { opSet(0, B); } 

void Cpu::opCb_0xc1() { opSet(0, C) ;} 

void Cpu::opCb_0xc2() { opSet(0, D) ;} 

void Cpu::opCb_0xc3() { opSet(0, E) ;} 

void Cpu::opCb_0xc4() { opSet(0, H) ;} 

void Cpu::opCb_0xc5() { opSet(0, L) ;} 

void Cpu::opCb_0xc6() { opSet(0, memory[getHL()]); } 

void Cpu::opCb_0xc7() { opSet(0, A); } 

void Cpu::opCb_0xc8() { opSet(1, B); } 

void Cpu::opCb_0xc9() { opSet(1, C); } 

void Cpu::opCb_0xca() { opSet(1, D); } 

void Cpu::opCb_0xcb() { opSet(1, E); } 

void Cpu::opCb_0xcc() { opSet(1, H); } 

void Cpu::opCb_0xcd() { opSet(1, L); } 

void Cpu::opCb_0xce() { opSet(1, memory[getHL()]); } 

void Cpu::opCb_0xcf() { opSet(1, A); } 

void Cpu::opCb_0xd0() { opSet(2, B); } 

void Cpu::opCb_0xd1() { opSet(2, C); } 

void Cpu::opCb_0xd2() { opSet(2, D); } 

void Cpu::opCb_0xd3() { opSet(2, E); } 

void Cpu::opCb_0xd4() { opSet(2, H); } 

void Cpu::opCb_0xd5() { opSet(2, L); } 

void Cpu::opCb_0xd6() { opSet(2, memory[getHL()]); } 

void Cpu::opCb_0xd7() { opSet(2, A); } 

void Cpu::opCb_0xd8() { opSet(3, B); } 

void Cpu::opCb_0xd9() { opSet(3, C); } 

void Cpu::opCb_0xda() { opSet(3, D); } 

void Cpu::opCb_0xdb() { opSet(3, E); } 

void Cpu::opCb_0xdc() { opSet(3, H); } 

void Cpu::opCb_0xdd() { opSet(3, L); } 

void Cpu::opCb_0xde() { opSet(3, memory[getHL()]); } 

void Cpu::opCb_0xdf() { opSet(3, A); }  

void Cpu::opCb_0xe0() { opSet(4, B); } 

void Cpu::opCb_0xe1() { opSet(4, C); } 

void Cpu::opCb_0xe2() { opSet(4, D); } 

void Cpu::opCb_0xe3() { opSet(4, E); } 

void Cpu::opCb_0xe4() { opSet(4, H); } 

void Cpu::opCb_0xe5() { opSet(4, L); } 

void Cpu::opCb_0xe6() { opSet(4, memory[getHL()]); } 

void Cpu::opCb_0xe7() { opSet(4, A); } 

void Cpu::opCb_0xe8() { opSet(5, B); } 

void Cpu::opCb_0xe9() { opSet(5, C); } 

void Cpu::opCb_0xea() { opSet(5, D); } 

void Cpu::opCb_0xeb() { opSet(5, E); } 

void Cpu::opCb_0xec() { opSet(5, H); } 

void Cpu::opCb_0xed() { opSet(5, L); } 

void Cpu::opCb_0xee() { opSet(5, memory[getHL()]); } 

void Cpu::opCb_0xef() { opSet(5, A); } 

void Cpu::opCb_0xf0() { opSet(6, B); } 

void Cpu::opCb_0xf1() { opSet(6, C); } 

void Cpu::opCb_0xf2() { opSet(6, D); } 

void Cpu::opCb_0xf3() { opSet(6, E); } 

void Cpu::opCb_0xf4() { opSet(6, H); } 

void Cpu::opCb_0xf5() { opSet(6, L); } 

void Cpu::opCb_0xf6() { opSet(6, memory[getHL()]); } 

void Cpu::opCb_0xf7() { opSet(6, A); } 

void Cpu::opCb_0xf8() { opSet(7, B); } 

void Cpu::opCb_0xf9() { opSet(7, C); } 

void Cpu::opCb_0xfa() { opSet(7, D); } 

void Cpu::opCb_0xfb() { opSet(7, E); } 

void Cpu::opCb_0xfc() { opSet(7, H); } 

void Cpu::opCb_0xfd() { opSet(7, L); } 

void Cpu::opCb_0xfe() { opSet(7, memory[getHL()]); } 

void Cpu::opCb_0xff() { opSet(7, A); }

