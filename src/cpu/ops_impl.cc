#include "cpu.h"
#include "scheduler.h"
#include "types.h"
#include "mem.h"

/////////////////////////////////////////////////////////////////
// Helper instructions
/////////////////////////////////////////////////////////////////


// load 
void Cpu::opLd(byte& dest, byte src) { dest = src; } 

void Cpu::opLd(word& dest, word src) { dest = src; }

void Cpu::opLd(word addr, byte src) { memory.write(addr, src); }

void Cpu::opLd(byte& dest_lo, byte& dest_hi, word src) 
{ 
    dest_hi = src >> 8;
    dest_lo = src;
}

// arithmetic

void Cpu::opAdd(byte val) 
{
    byte tmp = a;
    a += val;
    setCF(a < tmp); 
    setZF(!a); 
    setHF((a & 0b1111) < (tmp & 0b1111));
    setNF(false);
}

void Cpu::opAdd(word val) 
{
    word tmp = getHL();
    word new_val = tmp + val;
    setHL(new_val); 
    setCF(new_val < tmp); 
    setHF((new_val & 0b111111111111) < (tmp & 0b111111111111));
    setNF(false);
}

void Cpu::opAddSP(offs val)
{
    word tmp = sp;
    sp = tmp + val;
    setCF((byte) sp < (byte) tmp); 
    setHF((sp & 0b1111) < (tmp & 0b1111));
    f &= HF_MASK | CF_MASK;
}

void Cpu::opAdc(byte val) 
{
    byte tmp = a;
    bool cf = getCF();
    a += val + cf; 
    setCF((a < tmp) || (a == tmp && cf));
    setZF(!a);
    setHF
    (
        ((a & 0b1111) < (tmp & 0b1111)) || 
        ((a & 0b1111) == (tmp & 0b1111) && cf)
    );
    setNF(false);
}

void Cpu::opSub(byte val) 
{
    byte tmp = a;
    a -= val; 
    setCF(a > tmp); 
    setZF(!a); 
    setHF((a & 0b1111) > (tmp & 0b1111));
    setNF(true);
}

void Cpu::opSbc(byte val) 
{
    byte tmp = a;
    bool cf = getCF();
    a -= val + cf; 
    setCF((tmp < a) || (a == tmp && cf)); 
    setZF(!a); 
    setHF
    (
        ((a & 0b1111) > (tmp & 0b1111)) || 
        ((a & 0b1111) == (tmp & 0b1111) && cf)
    );
    setNF(true);
}


void Cpu::opCp(byte val) 
{
    byte tmp = a - val;
    setCF(a < tmp); 
    setZF(!tmp); 
    setHF((a & 0b1111) < (tmp & 0b1111));
    setNF(true);
}

void Cpu::opInc(byte& dest) 
{
    byte tmp = dest++;
    setZF(!dest); 
    setHF((dest & 0b1111) < (tmp & 0b1111));
    setNF(false);
}

void Cpu::opInc(word& dest) { ++dest; }

void Cpu::opInc(byte& lo, byte& hi) { hi += !(++lo); }

void Cpu::opIncMemory(unsigned short addr) 
{ 
    byte old_val = memory.read(addr);
    byte new_val = old_val + 1;
    memory.write(addr, new_val); 
    setZF(!new_val); 
    setHF((new_val & 0b1111) < (old_val & 0b1111));
    setNF(false);
}

void Cpu::opDec(byte& dest)
{
    byte tmp = dest--;
    setZF(!dest); 
    setHF((dest & 0b1111) > (tmp & 0b1111));
    setNF(true);
}

void Cpu::opDec(word& dest) { --dest; }

void Cpu::opDec(byte& lo, byte& hi) { hi -= (--lo == (byte)(-1)); }

void Cpu::opDecMemory(unsigned short addr) 
{
    byte old_val = memory.read(addr);
    byte new_val = old_val - 1;
    memory.write(addr, new_val); 
    setZF(!new_val); 
    setHF((new_val & 0b1111) > (old_val & 0b1111));
    setNF(true);

}


// logic

void Cpu::opAnd(byte val) 
{
    a &= val; 
    f &= NULLF_MASK;
    setHF(true);
    setZF(!a); 
}

void Cpu::opOr(byte val) 
{
    a |= val; 
    f &= NULLF_MASK;
    setZF(!a); 
}

void Cpu::opXor(byte val)
{
    a ^= val; 
    f &= NULLF_MASK;
    setZF(!a); 
}

void Cpu::opCpl()
{
    a = ~a;
    f |= 0b01100000;
}

//bit
void Cpu::opBit(byte shift, byte val) 
{
    setZF(!(val & (1u << shift)));
    setNF(false);
    setHF(true);
}

void Cpu::opRes(byte shift, byte& dest) { dest &= ~(1u << shift); } 

void Cpu::opRes(byte shift, unsigned short addr) { memory.write(addr, memory.read(addr) & ~(1u << shift)); } 

void Cpu::opSet(byte shift, byte& dest) { dest |= (1u << shift); }

void Cpu::opSet(byte shift, unsigned short addr) { memory.write(addr, memory.read(addr) | (1u << shift)); }

// bitshift

void Cpu::opRl(byte& dest) 
{
    bool new_CF = dest & 0b10000000;
    dest = (dest << 1) + getCF();
    setCF(new_CF);
    setZF(!dest);
    f &= ZF_MASK | CF_MASK;
}

void Cpu::opRl(unsigned short addr)
{
    byte res = memory.read(addr);
    bool new_CF = res & 0b10000000;
    res = (res << 1) + getCF();
    setCF(new_CF);
    setZF(!res);
    f &= ZF_MASK | CF_MASK;
    memory.write(addr, res);
}

void Cpu::opRla()
{
    bool new_CF = a & 0b10000000;
    a = (a << 1) + getCF();
    setCF(new_CF);
    setZF(!a);
    f &= CF_MASK;
}

void Cpu::opRlc(byte& dest)
{
    bool last_set = dest & 0b10000000;
    dest = (dest << 1) + last_set;
    setCF(last_set);
    setZF(!dest);
    f &= ZF_MASK | CF_MASK;
}

void Cpu::opRlc(unsigned short addr)
{
    byte res = memory.read(addr);
    bool last_set = res & 0b10000000;
    res = (res << 1) + last_set;
    setCF(last_set);
    setZF(!res);
    f &= ZF_MASK | CF_MASK;
    memory.write(addr, res);
}

void Cpu::opRlca()
{
    bool last_set = a & 0b10000000;
    a = (a << 1) + last_set;
    setCF(last_set);
    setZF(!a);
    f &= CF_MASK;
}

void Cpu::opRr(byte& dest)
{
    bool new_CF = dest & 0b00000001;
    dest = (dest >> 1) + getCF() * 0b10000000;
    setCF(new_CF);
    setZF(!dest);
    f &= ZF_MASK | CF_MASK;
}

void Cpu::opRr(unsigned short addr)
{
    byte res = memory.read(addr);
    bool new_CF = res & 0b00000001;
    res = (res >> 1) + getCF() * 0b10000000;
    setCF(new_CF);
    setZF(!res);
    f &= ZF_MASK | CF_MASK;
    memory.write(addr, res);
}

void Cpu::opRra()
{
    bool new_CF = a & 0b00000001;
    a = (a >> 1) + getCF() * 0b10000000;
    setCF(new_CF);
    setZF(!a);
    f &= CF_MASK;
}

void Cpu::opRrc(byte& dest)
{
    bool last_set = dest & 0b00000001;
    dest = (dest >> 1) + last_set * 0b10000000;
    setCF(last_set);
    setZF(!dest);
    f &= ZF_MASK | CF_MASK;
}

void Cpu::opRrc(unsigned short addr)
{
    byte res = memory.read(addr);
    bool last_set = res & 0b00000001;
    res = (res >> 1) + last_set * 0b10000000;
    setCF(last_set);
    setZF(!res);
    f &= ZF_MASK | CF_MASK;
    memory.write(addr, res);
}

void Cpu::opRrca()
{
    bool lowest_bit = a & 0b00000001;
    a = (a >> 1) + lowest_bit * 0b10000000;
    setCF(lowest_bit);
    setZF(!a);
    f &= CF_MASK;
}

void Cpu::opSla(byte& dest)
{
    setCF(dest & 0b10000000);
    dest <<= 1;
    setZF(!dest);
    f &= ZF_MASK | CF_MASK;
}

void Cpu::opSla(unsigned short addr)
{
    byte res = memory.read(addr);
    setCF(res & 0b10000000);
    res <<= 1;
    setZF(!res);
    f &= ZF_MASK | CF_MASK;
    memory.write(addr, res);
}

void Cpu::opSra(byte& dest)
{
    setCF(dest & 0b00000001u);
    dest = (dest & 0b10000000) + (dest >> 1);
    setZF(!dest);
    f &= ZF_MASK | CF_MASK;
}

void Cpu::opSra(unsigned short addr)
{
    byte res = memory.read(addr);
    setCF(res & 0b00000001u);
    res = (res & 0b10000000) + (res >> 1);
    setZF(!res);
    f &= ZF_MASK | CF_MASK;
    memory.write(addr, res);
}

void Cpu::opSrl(byte& dest)
{
    setCF(dest & 0b00000001);
    dest >>= 1;
    setZF(!dest);
    f &= ZF_MASK | CF_MASK;
}

void Cpu::opSrl(unsigned short addr)
{
    byte res = memory.read(addr);
    setCF(res & 0b00000001);
    res >>= 1;
    setZF(!res);
    f &= ZF_MASK | CF_MASK;
    memory.write(addr, res);
}

void Cpu::opSwap(byte& dest)
{
    dest = (dest << 4) + (dest >> 4);
    setZF(!dest);
    f &= ZF_MASK;
}

void Cpu::opSwap(unsigned short addr)
{
    byte res = memory.read(addr);
    res = (res << 4) + (res >> 4);
    setZF(!res);
    f &= ZF_MASK;
    memory.write(addr, res);
}


// jumps and subroutine

void Cpu::opCall(word addr)
{
    opPush(pc + 3);   
    pc = addr;
}

int Cpu::opCall(bool jump, word addr)
{
    if (jump) 
    {
        opCall(addr);
        return 6 * CLOCKS_BETWEEN_EXEC;
    }
    programCounterStep(3);
    return 3 * CLOCKS_BETWEEN_EXEC;
}

void Cpu::opJp(word addr) { pc = addr; }

int  Cpu::opJp(bool jump, word addr)
{
    if (jump) 
    {
        opJp(addr);
        return 4 * CLOCKS_BETWEEN_EXEC;
    }
    programCounterStep(3);
    return 3 * CLOCKS_BETWEEN_EXEC;
}

void Cpu::opJr(offs addr) { pc += addr; }

int  Cpu::opJr(bool jump, offs addr)
{
    u8 time = C(2);
    if (jump)
    {
        opJr(addr);
        time = C(3);
    }
    programCounterStep(2);
    return time;
}

void Cpu::opRet() { opPop(pc); }

int  Cpu::opRet(bool jump)
{
    if (jump)
    {
        opRet();
        return 5 * CLOCKS_BETWEEN_EXEC;
    }
    programCounterStep(1); 
    return 2 * CLOCKS_BETWEEN_EXEC;
}

void Cpu::opReti()
{
    opRet();
    opEi();
}

int  Cpu::opRst(u8 addr) 
{ 
    opPush(pc + 1);   
    pc = addr;
    return 4 * CLOCKS_BETWEEN_EXEC;
}

// carry flag

void Cpu::opCcf()
{
    setCF(!getCF());
    setNF(false);
    setHF(false);
}

void Cpu::opScf()
{
    setCF(true);
    f &= ZF_MASK | CF_MASK;
}

// stack manipulation

void Cpu::opAdd(offs val)
{
    word tmp = sp;
    sp += val;
    f &= HF_MASK | CF_MASK;
    setCF((sp & 0b11111111) < (tmp & 0b11111111)); 
    setHF((sp & 0b1111) < (tmp & 0b1111));
}

void Cpu::opPop(word& reg)
{
    reg = memory(sp);
    stack2StepBack();
}

void Cpu::opPop(byte& reg_lo, byte& reg_hi)
{
    reg_lo = memory.read(sp);
    reg_hi = memory.read(sp + 1);
    stack2StepBack();
}

void Cpu::opPush(word reg)
{
    memory.write(sp - 1, reg >> 8);
    memory.write(sp - 2, reg);
    stack2Step();
}

void Cpu::opDi() { ime = false; }

void Cpu::opEi() { ime = true; }

void Cpu::opHalt() { halted = true; }

// misc  

void Cpu::opDaa()
{
    byte tmp = a;

    if (getNF()) 
    {
        if (getHF())
            a -= 0x6;
        if (getCF())
            a -= 0x60;
    
        setZF(!a);
        setHF(false);

        return;
    }
    if (getHF() || ((tmp & 0xf) > 0x9))
        a += 0x6;
    if (getCF() || (tmp > 0x99))
    {
        setCF(true);
        a += 0x60;
    }

    setHF(false);
    setZF(!a);
}

void Cpu::opNop() {}

void Cpu::opStop() {/*most likely not gonna use*/}


/////////////////////////////////////////////////////////////////
// Regular (8 bit) operations
/////////////////////////////////////////////////////////////////

// some instructions have variable cycles, so they might insert the next instruction, instead of the main exexecute function, also progress the PC further
void Cpu::op_0x00() { opNop(); } 

void Cpu::op_0x01() { opLd(c, b, memory(pc + 1)); } 

void Cpu::op_0x02() 
{ 
    unsigned short addr = getBC();
    opLd(addr, a); 
} 

void Cpu::op_0x03() { opInc(c, b); } 

void Cpu::op_0x04() { opInc(b); } 

void Cpu::op_0x05() { opDec(b); } 

void Cpu::op_0x06() { opLd(b, memory.read(pc + 1)); } 

void Cpu::op_0x07() { opRlca(); } 

void Cpu::op_0x08() 
{
    unsigned short addr = memory(pc + 1);
  
    memory.write(addr + 1, sp >> 8);
    memory.write(addr, sp);
} 

void Cpu::op_0x09() { opAdd(getBC()); } 

void Cpu::op_0x0a() { opLd(a, memory.read(getBC())); } 

void Cpu::op_0x0b() { opDec(c, b); } 

void Cpu::op_0x0c() { opInc(c); } 

void Cpu::op_0x0d() { opDec(c); } 

void Cpu::op_0x0e() { opLd(c, memory.read(pc + 1)); } 

void Cpu::op_0x0f() { opRrca(); } 

void Cpu::op_0x10() { opStop(); } 

void Cpu::op_0x11() { opLd(e, d, memory(pc + 1)); } 

void Cpu::op_0x12() 
{ 
    unsigned short addr = getDE();
    opLd(addr, a); 
} 

void Cpu::op_0x13() { opInc(e, d); } 

void Cpu::op_0x14() { opInc(d); } 

void Cpu::op_0x15() { opDec(d); } 

void Cpu::op_0x16() { opLd(d, memory.read(pc + 1)); } 

void Cpu::op_0x17() { opRla(); } 

void Cpu::op_0x18() { opJr(memory.read(pc + 1)); } 

void Cpu::op_0x19() { opAdd(getDE()); } 

void Cpu::op_0x1a() { opLd(a, memory.read(getDE())); } 

void Cpu::op_0x1b() { opDec(e, d); } 

void Cpu::op_0x1c() { opInc(e); } 

void Cpu::op_0x1d() { opDec(e); } 

void Cpu::op_0x1e() { opLd(e, memory.read(pc + 1)); } 

void Cpu::op_0x1f() { opRra(); } 

int  Cpu::op_0x20() { return opJr(!getZF(), memory.read(pc + 1)); } 

void Cpu::op_0x21() { opLd(l, h, memory(pc + 1)); } 

void Cpu::op_0x22()
{ 
    unsigned short addr = getHL();
    opLd(addr, a);
    opInc(l, h);
} 

void Cpu::op_0x23() { opInc(l, h); } 

void Cpu::op_0x24() { opInc(h); } 

void Cpu::op_0x25() { opDec(h); } 

void Cpu::op_0x26() { opLd(h, memory.read(pc + 1)); } 

void Cpu::op_0x27() { opDaa(); } 

int  Cpu::op_0x28() { return opJr(getZF(), memory.read(pc + 1)); }

void Cpu::op_0x29() { opAdd(getHL()); } 

void Cpu::op_0x2a() 
{ 
    opLd(a, memory.read(getHL())); 
    opInc(l, h);
} 

void Cpu::op_0x2b() { opDec(l, h); } 

void Cpu::op_0x2c() { opInc(l); } 

void Cpu::op_0x2d() { opDec(l); } 

void Cpu::op_0x2e() { opLd(l, memory.read(pc + 1)); } 

void Cpu::op_0x2f() { opCpl(); } 

int  Cpu::op_0x30() { return opJr(!getCF(), memory.read(pc + 1)); } 

void Cpu::op_0x31() { opLd(sp, memory(pc + 1)); } 

void Cpu::op_0x32() 
{ 
    unsigned short addr = getHL();
    opLd(addr, a);
    opDec(l, h);
} 

void Cpu::op_0x33() { opInc(sp); } 

void Cpu::op_0x34() 
{ 
    unsigned short addr = getHL();
    opIncMemory(addr);
} 

void Cpu::op_0x35() 
{ 
    unsigned short addr = getHL();
    opDecMemory(addr);
} 

void Cpu::op_0x36() 
{
    unsigned short addr = getHL();
    opLd(addr, memory.read(pc + 1)); 
} 

void Cpu::op_0x37() { opScf(); } 

int  Cpu::op_0x38() { return opJr(getCF(), memory.read(pc + 1)); } 

void Cpu::op_0x39() { opAdd(sp); } 

void Cpu::op_0x3a() 
{
    opLd(a, memory.read(getHL()));
    opDec(l, h);
} 

void Cpu::op_0x3b() { opDec(sp); } 

void Cpu::op_0x3c() { opInc(a); } 

void Cpu::op_0x3d() { opDec(a); } 

void Cpu::op_0x3e() { opLd(a, memory.read(pc + 1)); } 

void Cpu::op_0x3f() { opCcf(); } 

void Cpu::op_0x40() { opLd(b, b); } 

void Cpu::op_0x41() { opLd(b, c); } 

void Cpu::op_0x42() { opLd(b, d); } 

void Cpu::op_0x43() { opLd(b, e); } 

void Cpu::op_0x44() { opLd(b, h); } 

void Cpu::op_0x45() { opLd(b, l); } 

void Cpu::op_0x46() { opLd(b, memory.read(getHL()));  } 

void Cpu::op_0x47() { opLd(b, a); } 

void Cpu::op_0x48() { opLd(c, b); } 

void Cpu::op_0x49() { opLd(c, c); } 

void Cpu::op_0x4a() { opLd(c, d); } 

void Cpu::op_0x4b() { opLd(c, e); } 

void Cpu::op_0x4c() { opLd(c, h); } 

void Cpu::op_0x4d() { opLd(c, l); } 

void Cpu::op_0x4e() { opLd(c, memory.read(getHL())); } 

void Cpu::op_0x4f() { opLd(c, a); } 

void Cpu::op_0x50() { opLd(d, b); } 

void Cpu::op_0x51() { opLd(d, c); } 

void Cpu::op_0x52() { opLd(d, d); } 

void Cpu::op_0x53() { opLd(d, e); } 

void Cpu::op_0x54() { opLd(d, h); } 

void Cpu::op_0x55() { opLd(d, l); } 

void Cpu::op_0x56() { opLd(d, memory.read(getHL())); } 

void Cpu::op_0x57() { opLd(d, a); } 

void Cpu::op_0x58() { opLd(e, b); } 

void Cpu::op_0x59() { opLd(e, c); } 

void Cpu::op_0x5a() { opLd(e, d); } 

void Cpu::op_0x5b() { opLd(e, e); } 

void Cpu::op_0x5c() { opLd(e, h); } 

void Cpu::op_0x5d() { opLd(e, l); } 

void Cpu::op_0x5e() { opLd(e, memory.read(getHL())); } 

void Cpu::op_0x5f() { opLd(e, a); } 

void Cpu::op_0x60() { opLd(h, b); } 

void Cpu::op_0x61() { opLd(h, c); } 

void Cpu::op_0x62() { opLd(h, d); } 

void Cpu::op_0x63() { opLd(h, e); } 

void Cpu::op_0x64() { opLd(h, h); } 

void Cpu::op_0x65() { opLd(h, l); } 

void Cpu::op_0x66() { opLd(h, memory.read(getHL())); } 

void Cpu::op_0x67() { opLd(h, a); } 

void Cpu::op_0x68() { opLd(l, b); } 

void Cpu::op_0x69() { opLd(l, c); } 

void Cpu::op_0x6a() { opLd(l, d); } 

void Cpu::op_0x6b() { opLd(l, e); } 

void Cpu::op_0x6c() { opLd(l, h); } 

void Cpu::op_0x6d() { opLd(l, l); } 

void Cpu::op_0x6e() { opLd(l, memory.read(getHL())); } 

void Cpu::op_0x6f() { opLd(l, a); } 

void Cpu::op_0x70() 
{
    unsigned short addr = getHL();
    opLd(addr, b); 
} 

void Cpu::op_0x71() 
{
    unsigned short addr = getHL();
    opLd(addr, c); 
} 

void Cpu::op_0x72() 
{
    unsigned short addr = getHL();
    opLd(addr, d);  
} 
void Cpu::op_0x73() 
{
    unsigned short addr = getHL();
    opLd(addr, e); 
} 

void Cpu::op_0x74() 
{
    unsigned short addr = getHL();
    opLd(addr, h);
} 

void Cpu::op_0x75() 
{
    unsigned short addr = getHL();
    opLd(addr, l); 
} 

void Cpu::op_0x76() { opHalt(); } 

void Cpu::op_0x77() 
{ 
    unsigned short addr = getHL();
    opLd(addr, a); 
} 

void Cpu::op_0x78() { opLd(a, b); } 

void Cpu::op_0x79() { opLd(a, c); } 

void Cpu::op_0x7a() { opLd(a, d); } 

void Cpu::op_0x7b() { opLd(a, e); } 

void Cpu::op_0x7c() { opLd(a, h); } 

void Cpu::op_0x7d() { opLd(a, l); } 

void Cpu::op_0x7e() { opLd(a, memory.read(getHL())); } 

void Cpu::op_0x7f() { opLd(a, a); } 

void Cpu::op_0x80() { opAdd(b); } 

void Cpu::op_0x81() { opAdd(c); } 

void Cpu::op_0x82() { opAdd(d); } 

void Cpu::op_0x83() { opAdd(e); } 

void Cpu::op_0x84() { opAdd(h); } 

void Cpu::op_0x85() { opAdd(l); } 

void Cpu::op_0x86() { opAdd(memory.read(getHL())); } 

void Cpu::op_0x87() { opAdd(a); } 

void Cpu::op_0x88() { opAdc(b); } 

void Cpu::op_0x89() { opAdc(c); } 

void Cpu::op_0x8a() { opAdc(d); } 

void Cpu::op_0x8b() { opAdc(e); } 

void Cpu::op_0x8c() { opAdc(h); } 

void Cpu::op_0x8d() { opAdc(l); } 

void Cpu::op_0x8e() { opAdc(memory.read(getHL())); } 

void Cpu::op_0x8f() { opAdc(a); } 

void Cpu::op_0x90() { opSub(b); } 

void Cpu::op_0x91() { opSub(c); } 

void Cpu::op_0x92() { opSub(d); } 

void Cpu::op_0x93() { opSub(e); } 

void Cpu::op_0x94() { opSub(h); } 

void Cpu::op_0x95() { opSub(l); } 

void Cpu::op_0x96() { opSub(memory.read(getHL())); } 

void Cpu::op_0x97() { opSub(a); } 

void Cpu::op_0x98() { opSbc(b); } 

void Cpu::op_0x99() { opSbc(c); } 

void Cpu::op_0x9a() { opSbc(d); } 

void Cpu::op_0x9b() { opSbc(e); } 

void Cpu::op_0x9c() { opSbc(h); } 

void Cpu::op_0x9d() { opSbc(l); } 

void Cpu::op_0x9e() { opSbc(memory.read(getHL())); } 

void Cpu::op_0x9f() { opSbc(a); } 

void Cpu::op_0xa0() { opAnd(b); } 

void Cpu::op_0xa1() { opAnd(c); } 

void Cpu::op_0xa2() { opAnd(d); } 

void Cpu::op_0xa3() { opAnd(e); } 

void Cpu::op_0xa4() { opAnd(h); } 

void Cpu::op_0xa5() { opAnd(l); } 

void Cpu::op_0xa6() { opAnd(memory.read(getHL())); } 

void Cpu::op_0xa7() { opAnd(a); } 

void Cpu::op_0xa8() { opXor(b); } 

void Cpu::op_0xa9() { opXor(c); } 

void Cpu::op_0xaa() { opXor(d); } 

void Cpu::op_0xab() { opXor(e); } 

void Cpu::op_0xac() { opXor(h); } 

void Cpu::op_0xad() { opXor(l); } 

void Cpu::op_0xae() { opXor(memory.read(getHL())); } 

void Cpu::op_0xaf() { opXor(a); } 

void Cpu::op_0xb0() { opOr(b); } 

void Cpu::op_0xb1() { opOr(c); } 

void Cpu::op_0xb2() { opOr(d); } 

void Cpu::op_0xb3() { opOr(e); } 

void Cpu::op_0xb4() { opOr(h); } 

void Cpu::op_0xb5() { opOr(l); } 

void Cpu::op_0xb6() { opOr(memory.read(getHL())); } 

void Cpu::op_0xb7() { opOr(a); } 

void Cpu::op_0xb8() { opCp(b); } 

void Cpu::op_0xb9() { opCp(c); } 

void Cpu::op_0xba() { opCp(d); } 

void Cpu::op_0xbb() { opCp(e); } 

void Cpu::op_0xbc() { opCp(h); } 

void Cpu::op_0xbd() { opCp(l); } 

void Cpu::op_0xbe() { opCp(memory.read(getHL())); } 

void Cpu::op_0xbf() { opCp(a); } 

int  Cpu::op_0xc0() { return opRet(!getZF()); } 

void Cpu::op_0xc1() { opPop(c, b); } 

int  Cpu::op_0xc2() { return opJp(!getZF(), memory(pc + 1)); } 

int  Cpu::op_0xc3() 
{ 
    opJp(memory(pc + 1)); 
    return 4 * CLOCKS_BETWEEN_EXEC;
} 

int  Cpu::op_0xc4() { return opCall(!getZF(), memory(pc + 1)); } 

void Cpu::op_0xc5() { opPush(getBC()); } 

void Cpu::op_0xc6() { opAdd(memory.read(pc + 1)); } 

int  Cpu::op_0xc7() { return opRst(8 * 0); } 

int  Cpu::op_0xc8() { return opRet(getZF()); } 

// This is not how it should be, but it'll make do
void Cpu::op_0xc9() 
{ 
    opRet(); 
    pc -= 1;
} 

int Cpu::op_0xca() { return opJp(getZF(), memory(pc + 1)); } 

int  Cpu::op_0xcb() { return executeBC(memory.read(pc + 1)); } 

int  Cpu::op_0xcc() { return opCall(getZF(), memory(pc + 1)); } 

int  Cpu::op_0xcd() { return opCall(true, memory(pc + 1)); } 

void Cpu::op_0xce() { opAdc(memory.read(pc + 1)); } 

int  Cpu::op_0xcf() { return opRst(8 * 1); } 

int  Cpu::op_0xd0() { return opRet(!getCF()); } 

void Cpu::op_0xd1() { opPop(e, d); } 

int  Cpu::op_0xd2() { return opJp(!getCF(), memory(pc + 1)); } 

void Cpu::op_0xd3() { opNop(); }   // I'm gonna be generous, and not crash the console when it finds an unknown op code 

int  Cpu::op_0xd4() { return opCall(!getCF(), memory(pc + 1)); } 

void Cpu::op_0xd5() { opPush(getDE()); } 

void Cpu::op_0xd6() { opSub(memory.read(pc + 1)); } 

int  Cpu::op_0xd7() { return opRst(8 * 2); } 

int  Cpu::op_0xd8() { return opRet(getCF()); } 

void Cpu::op_0xd9() 
{ 
    opReti();
    pc -= 1;
} 

int  Cpu::op_0xda() { return opJp(getCF(), memory(pc + 1)); } 

void Cpu::op_0xdb() { opNop(); } 

int  Cpu::op_0xdc() { return opCall(getCF(), memory(pc + 1)); } 

void Cpu::op_0xdd() { opNop(); } 

void Cpu::op_0xde() { opSbc(memory.read(pc + 1)); } 

int  Cpu::op_0xdf() { return opRst(8 * 3); } 

void Cpu::op_0xe0() 
{
    unsigned short addr = 0xff00 + memory.read(pc + 1);
    opLd(addr, a); 
} 

void Cpu::op_0xe1() { opPop(l, h); } 

void Cpu::op_0xe2() 
{ 
    unsigned short addr = 0xff00 + c;
    opLd(addr, a); 
} 

void Cpu::op_0xe3() { opNop(); } 

void Cpu::op_0xe4() { opNop(); } 

void Cpu::op_0xe5() { opPush(getHL()); } 

void Cpu::op_0xe6() { opAnd(memory.read(pc + 1)); } 

int  Cpu::op_0xe7() { return opRst(8 * 4); } 

void Cpu::op_0xe8() { opAddSP(memory.read(pc + 1)); } 

int  Cpu::op_0xe9() 
{
    opJp(getHL()); 
    return 1 * CLOCKS_BETWEEN_EXEC;
} 

void Cpu::op_0xea() 
{
    unsigned short addr = memory(pc + 1);
    opLd(addr, a);
} 

void Cpu::op_0xeb() { opNop(); } 

void Cpu::op_0xec() { opNop(); } 

void Cpu::op_0xed() { opNop(); } 

void Cpu::op_0xee() { opXor(memory.read(pc + 1)); } 

int  Cpu::op_0xef() { return opRst(8 * 5); } 

void Cpu::op_0xf0() { opLd(a, memory.read(0xff00 + memory.read(pc + 1)));} 

void Cpu::op_0xf1() 
{ 
    opPop(f, a);
    f &= 0b11110000;
} 

void Cpu::op_0xf2() { opLd(a, memory.read(0xff00 + c)); } 

void Cpu::op_0xf3() { opDi(); } 

void Cpu::op_0xf4() { opNop(); } 

void Cpu::op_0xf5() { opPush(getAF()); } 

void Cpu::op_0xf6() { opOr(memory.read(pc + 1)); } 

int  Cpu::op_0xf7() { return opRst(8 * 6); } 

void Cpu::op_0xf8() 
{ 
    word tmp = sp + (offs) memory.read(pc + 1);
    opLd(l, h, tmp); 
    setCF((byte) sp > (byte) tmp); 
    setHF((sp & 0b1111) > (tmp & 0b1111));
    f &= HF_MASK | CF_MASK;
} 

void Cpu::op_0xf9() { opLd(sp, getHL()); } 

void Cpu::op_0xfa() { opLd(a, memory.read(memory(pc + 1))); } 

void Cpu::op_0xfb() { opEi(); } 

void Cpu::op_0xfc() { opNop(); } 

void Cpu::op_0xfd() { opNop(); } 

void Cpu::op_0xfe() { opCp(memory.read(pc + 1)); } 

int  Cpu::op_0xff() { return opRst(8 * 7); } 


/////////////////////////////////////////////////////////////////
// bc (16 bit) operations
/////////////////////////////////////////////////////////////////


void Cpu::opCb_0x00() { opRlc(b); } 

void Cpu::opCb_0x01() { opRlc(c); } 

void Cpu::opCb_0x02() { opRlc(d); } 

void Cpu::opCb_0x03() { opRlc(e); } 

void Cpu::opCb_0x04() { opRlc(h); } 

void Cpu::opCb_0x05() { opRlc(l); } 

void Cpu::opCb_0x06() 
{
    unsigned short addr = getHL();
    opRlc(addr); 
} 

void Cpu::opCb_0x07() { opRlc(a); } 

void Cpu::opCb_0x08() { opRrc(b); } 

void Cpu::opCb_0x09() { opRrc(c); } 

void Cpu::opCb_0x0a() { opRrc(d); } 

void Cpu::opCb_0x0b() { opRrc(e); } 

void Cpu::opCb_0x0c() { opRrc(h); } 

void Cpu::opCb_0x0d() { opRrc(l); } 

void Cpu::opCb_0x0e() 
{
    unsigned short addr = getHL();
    opRrc(addr);
} 

void Cpu::opCb_0x0f() { opRrc(a); } 

void Cpu::opCb_0x10() { opRl(b); } 

void Cpu::opCb_0x11() { opRl(c); } 

void Cpu::opCb_0x12() { opRl(d); } 

void Cpu::opCb_0x13() { opRl(e); } 

void Cpu::opCb_0x14() { opRl(h); } 

void Cpu::opCb_0x15() { opRl(l); } 

void Cpu::opCb_0x16() 
{ 
    unsigned short addr = getHL();
    opRl(addr); 
} 

void Cpu::opCb_0x17() { opRl(a); } 

void Cpu::opCb_0x18() { opRr(b); } 

void Cpu::opCb_0x19() { opRr(c); } 

void Cpu::opCb_0x1a() { opRr(d); } 

void Cpu::opCb_0x1b() { opRr(e); } 

void Cpu::opCb_0x1c() { opRr(h); } 

void Cpu::opCb_0x1d() { opRr(l); } 

void Cpu::opCb_0x1e() 
{ 
    unsigned short addr = getHL();
    opRr(addr); 
} 

void Cpu::opCb_0x1f() { opRr(a); } 

void Cpu::opCb_0x20() { opSla(b); } 

void Cpu::opCb_0x21() { opSla(c); } 

void Cpu::opCb_0x22() { opSla(d); } 

void Cpu::opCb_0x23() { opSla(e); } 

void Cpu::opCb_0x24() { opSla(h); } 

void Cpu::opCb_0x25() { opSla(l); } 

void Cpu::opCb_0x26() 
{ 
    unsigned short addr = getHL();
    opSla(addr); 
} 

void Cpu::opCb_0x27() { opSla(a); } 

void Cpu::opCb_0x28() { opSra(b); } 

void Cpu::opCb_0x29() { opSra(c); } 

void Cpu::opCb_0x2a() { opSra(d); } 

void Cpu::opCb_0x2b() { opSra(e); } 

void Cpu::opCb_0x2c() { opSra(h); } 

void Cpu::opCb_0x2d() { opSra(l); } 

void Cpu::opCb_0x2e() 
{ 
    unsigned short addr = getHL();
    opSra(addr); 
} 

void Cpu::opCb_0x2f() { opSra(a); } 

void Cpu::opCb_0x30() { opSwap(b); } 

void Cpu::opCb_0x31() { opSwap(c); } 

void Cpu::opCb_0x32() { opSwap(d); } 

void Cpu::opCb_0x33() { opSwap(e); } 

void Cpu::opCb_0x34() { opSwap(h); } 

void Cpu::opCb_0x35() { opSwap(l); } 

void Cpu::opCb_0x36() 
{ 
    unsigned short addr = getHL();
    opSwap(addr); 
} 

void Cpu::opCb_0x37() { opSwap(a); } 

void Cpu::opCb_0x38() { opSrl(b); } 

void Cpu::opCb_0x39() { opSrl(c); } 

void Cpu::opCb_0x3a() { opSrl(d); } 

void Cpu::opCb_0x3b() { opSrl(e); } 

void Cpu::opCb_0x3c() { opSrl(h); } 

void Cpu::opCb_0x3d() { opSrl(l); } 

void Cpu::opCb_0x3e() 
{ 
    unsigned short addr = getHL();
    opSrl(addr); 
} 

void Cpu::opCb_0x3f() { opSrl(a); } 

void Cpu::opCb_0x40() { opBit(0, b); } 

void Cpu::opCb_0x41() { opBit(0, c); } 

void Cpu::opCb_0x42() { opBit(0, d); } 

void Cpu::opCb_0x43() { opBit(0, e); } 

void Cpu::opCb_0x44() { opBit(0, h); } 

void Cpu::opCb_0x45() { opBit(0, l); } 

void Cpu::opCb_0x46() { opBit(0, memory.read(getHL())); } 

void Cpu::opCb_0x47() { opBit(0, a); } 

void Cpu::opCb_0x48() { opBit(1, b); } 

void Cpu::opCb_0x49() { opBit(1, c); } 

void Cpu::opCb_0x4a() { opBit(1, d); } 

void Cpu::opCb_0x4b() { opBit(1, e); } 

void Cpu::opCb_0x4c() { opBit(1, h); } 

void Cpu::opCb_0x4d() { opBit(1, l); } 

void Cpu::opCb_0x4e() { opBit(1, memory.read(getHL())); } 

void Cpu::opCb_0x4f() { opBit(1, a); } 

void Cpu::opCb_0x50() { opBit(2, b); } 

void Cpu::opCb_0x51() { opBit(2, c); } 

void Cpu::opCb_0x52() { opBit(2, d); } 

void Cpu::opCb_0x53() { opBit(2, e); } 

void Cpu::opCb_0x54() { opBit(2, h); } 

void Cpu::opCb_0x55() { opBit(2, l); } 

void Cpu::opCb_0x56() { opBit(2, memory.read(getHL())); } 

void Cpu::opCb_0x57() { opBit(2, a); } 

void Cpu::opCb_0x58() { opBit(3, b); } 

void Cpu::opCb_0x59() { opBit(3, c); } 

void Cpu::opCb_0x5a() { opBit(3, d); } 

void Cpu::opCb_0x5b() { opBit(3, e); } 

void Cpu::opCb_0x5c() { opBit(3, h); } 

void Cpu::opCb_0x5d() { opBit(3, l); } 

void Cpu::opCb_0x5e() { opBit(3, memory.read(getHL())); } 

void Cpu::opCb_0x5f() { opBit(3, a); } 

void Cpu::opCb_0x60() { opBit(4, b); } 

void Cpu::opCb_0x61() { opBit(4, c); } 

void Cpu::opCb_0x62() { opBit(4, d); } 

void Cpu::opCb_0x63() { opBit(4, e); } 

void Cpu::opCb_0x64() { opBit(4, h); } 

void Cpu::opCb_0x65() { opBit(4, l); } 

void Cpu::opCb_0x66() { opBit(4, memory.read(getHL())); } 

void Cpu::opCb_0x67() { opBit(4, a); } 

void Cpu::opCb_0x68() { opBit(5, b); } 

void Cpu::opCb_0x69() { opBit(5, c); } 

void Cpu::opCb_0x6a() { opBit(5, d); } 

void Cpu::opCb_0x6b() { opBit(5, e); } 

void Cpu::opCb_0x6c() { opBit(5, h); } 

void Cpu::opCb_0x6d() { opBit(5, l); } 

void Cpu::opCb_0x6e() { opBit(5, memory.read(getHL())); } 

void Cpu::opCb_0x6f() { opBit(5, a); } 

void Cpu::opCb_0x70() { opBit(6, b); } 

void Cpu::opCb_0x71() { opBit(6, c); } 

void Cpu::opCb_0x72() { opBit(6, d); } 

void Cpu::opCb_0x73() { opBit(6, e); } 

void Cpu::opCb_0x74() { opBit(6, h); } 

void Cpu::opCb_0x75() { opBit(6, l); } 

void Cpu::opCb_0x76() { opBit(6, memory.read(getHL())); } 

void Cpu::opCb_0x77() { opBit(6, a); } 

void Cpu::opCb_0x78() { opBit(7, b); } 

void Cpu::opCb_0x79() { opBit(7, c); } 

void Cpu::opCb_0x7a() { opBit(7, d); } 

void Cpu::opCb_0x7b() { opBit(7, e); } 

void Cpu::opCb_0x7c() { opBit(7, h); } 

void Cpu::opCb_0x7d() { opBit(7, l); } 

void Cpu::opCb_0x7e() { opBit(7, memory.read(getHL())); } 

void Cpu::opCb_0x7f() { opBit(7, a); } 

void Cpu::opCb_0x80() { opRes(0, b); } 

void Cpu::opCb_0x81() { opRes(0, c); } 

void Cpu::opCb_0x82() { opRes(0, d); } 

void Cpu::opCb_0x83() { opRes(0, e); } 

void Cpu::opCb_0x84() { opRes(0, h); } 

void Cpu::opCb_0x85() { opRes(0, l); } 

void Cpu::opCb_0x86() 
{
    unsigned short addr = getHL();
    opRes(0, addr); 
} 

void Cpu::opCb_0x87() { opRes(0, a); } 

void Cpu::opCb_0x88() { opRes(1, b); } 

void Cpu::opCb_0x89() { opRes(1, c); } 

void Cpu::opCb_0x8a() { opRes(1, d); } 

void Cpu::opCb_0x8b() { opRes(1, e); } 

void Cpu::opCb_0x8c() { opRes(1, h); } 

void Cpu::opCb_0x8d() { opRes(1, l); } 

void Cpu::opCb_0x8e() 
{ 
    unsigned short addr = getHL();
    opRes(1, addr);
} 

void Cpu::opCb_0x8f() { opRes(1, a); } 

void Cpu::opCb_0x90() { opRes(2, b); } 

void Cpu::opCb_0x91() { opRes(2, c); } 

void Cpu::opCb_0x92() { opRes(2, d); } 

void Cpu::opCb_0x93() { opRes(2, e); } 

void Cpu::opCb_0x94() { opRes(2, h); } 

void Cpu::opCb_0x95() { opRes(2, l); } 

void Cpu::opCb_0x96()
{
    unsigned short addr = getHL();
    opRes(2, addr); 
} 

void Cpu::opCb_0x97() { opRes(2, a); } 

void Cpu::opCb_0x98() { opRes(3, b); } 

void Cpu::opCb_0x99() { opRes(3, c); } 

void Cpu::opCb_0x9a() { opRes(3, d); } 

void Cpu::opCb_0x9b() { opRes(3, e); } 

void Cpu::opCb_0x9c() { opRes(3, h); } 

void Cpu::opCb_0x9d() { opRes(3, l); } 

void Cpu::opCb_0x9e()
{ 
    unsigned short addr = getHL();
    opRes(3, addr); 
} 

void Cpu::opCb_0x9f() { opRes(3, a); } 

void Cpu::opCb_0xa0() { opRes(4, b); } 

void Cpu::opCb_0xa1() { opRes(4, c);} 

void Cpu::opCb_0xa2() { opRes(4, d); } 

void Cpu::opCb_0xa3() { opRes(4, e); } 

void Cpu::opCb_0xa4() { opRes(4, h); } 

void Cpu::opCb_0xa5() { opRes(4, l); } 

void Cpu::opCb_0xa6()
{ 
    unsigned short addr = getHL();
    opRes(4, addr); 
} 

void Cpu::opCb_0xa7() { opRes(4, a); } 

void Cpu::opCb_0xa8() { opRes(5, b); } 

void Cpu::opCb_0xa9() { opRes(5, c); } 

void Cpu::opCb_0xaa() { opRes(5, d); } 

void Cpu::opCb_0xab() { opRes(5, e); } 

void Cpu::opCb_0xac() { opRes(5, h); } 

void Cpu::opCb_0xad() { opRes(5, l); } 

void Cpu::opCb_0xae()
{
    unsigned short addr = getHL();
    opRes(5, addr); 
} 

void Cpu::opCb_0xaf() { opRes(5, a); } 

void Cpu::opCb_0xb0() { opRes(6, b); } 

void Cpu::opCb_0xb1() { opRes(6, c);  } 

void Cpu::opCb_0xb2() { opRes(6, d); } 

void Cpu::opCb_0xb3() { opRes(6, e);  } 

void Cpu::opCb_0xb4() { opRes(6, h); } 

void Cpu::opCb_0xb5() { opRes(6, l); } 

void Cpu::opCb_0xb6()
{
    unsigned short addr = getHL();
    opRes(6, addr); 
} 

void Cpu::opCb_0xb7() { opRes(6, a); } 

void Cpu::opCb_0xb8() { opRes(7, b); } 

void Cpu::opCb_0xb9() { opRes(7, c); } 

void Cpu::opCb_0xba() { opRes(7, d); } 

void Cpu::opCb_0xbb() { opRes(7, e); } 

void Cpu::opCb_0xbc() { opRes(7, h); } 

void Cpu::opCb_0xbd() { opRes(7, l); } 

void Cpu::opCb_0xbe()
{ 
    unsigned short addr = getHL();
    opRes(7, addr); 
} 

void Cpu::opCb_0xbf() { opRes(7, a); } 

void Cpu::opCb_0xc0() { opSet(0, b); } 

void Cpu::opCb_0xc1() { opSet(0, c) ;} 

void Cpu::opCb_0xc2() { opSet(0, d) ;} 

void Cpu::opCb_0xc3() { opSet(0, e) ;} 

void Cpu::opCb_0xc4() { opSet(0, h) ;} 

void Cpu::opCb_0xc5() { opSet(0, l) ;} 

void Cpu::opCb_0xc6() 
{
    unsigned short addr = getHL();
    opSet(0, addr); 
} 

void Cpu::opCb_0xc7() { opSet(0, a); } 

void Cpu::opCb_0xc8() { opSet(1, b); } 

void Cpu::opCb_0xc9() { opSet(1, c); } 

void Cpu::opCb_0xca() { opSet(1, d); } 

void Cpu::opCb_0xcb() { opSet(1, e); } 

void Cpu::opCb_0xcc() { opSet(1, h); } 

void Cpu::opCb_0xcd() { opSet(1, l); } 

void Cpu::opCb_0xce() 
{
    unsigned short addr = getHL();
    opSet(1, addr); 
} 

void Cpu::opCb_0xcf() { opSet(1, a); } 

void Cpu::opCb_0xd0() { opSet(2, b); } 

void Cpu::opCb_0xd1() { opSet(2, c); } 

void Cpu::opCb_0xd2() { opSet(2, d); } 

void Cpu::opCb_0xd3() { opSet(2, e); } 

void Cpu::opCb_0xd4() { opSet(2, h); } 

void Cpu::opCb_0xd5() { opSet(2, l); } 

void Cpu::opCb_0xd6() 
{
    unsigned short addr = getHL();
    opSet(2, addr); 
} 

void Cpu::opCb_0xd7() { opSet(2, a); } 

void Cpu::opCb_0xd8() { opSet(3, b); } 

void Cpu::opCb_0xd9() { opSet(3, c); } 

void Cpu::opCb_0xda() { opSet(3, d); } 

void Cpu::opCb_0xdb() { opSet(3, e); } 

void Cpu::opCb_0xdc() { opSet(3, h); } 

void Cpu::opCb_0xdd() { opSet(3, l); } 

void Cpu::opCb_0xde() 
{
    unsigned short addr = getHL();
    opSet(3, addr); 
} 

void Cpu::opCb_0xdf() { opSet(3, a); }  

void Cpu::opCb_0xe0() { opSet(4, b); } 

void Cpu::opCb_0xe1() { opSet(4, c); } 

void Cpu::opCb_0xe2() { opSet(4, d); } 

void Cpu::opCb_0xe3() { opSet(4, e); } 

void Cpu::opCb_0xe4() { opSet(4, h); } 

void Cpu::opCb_0xe5() { opSet(4, l); } 

void Cpu::opCb_0xe6()
{
    unsigned short addr = getHL();
    opSet(4, addr); 
} 

void Cpu::opCb_0xe7() { opSet(4, a); } 

void Cpu::opCb_0xe8() { opSet(5, b); } 

void Cpu::opCb_0xe9() { opSet(5, c); } 

void Cpu::opCb_0xea() { opSet(5, d); } 

void Cpu::opCb_0xeb() { opSet(5, e); } 

void Cpu::opCb_0xec() { opSet(5, h); } 

void Cpu::opCb_0xed() { opSet(5, l); } 

void Cpu::opCb_0xee()
{
    unsigned short addr = getHL();
    opSet(5, addr); 
} 

void Cpu::opCb_0xef() { opSet(5, a); } 

void Cpu::opCb_0xf0() { opSet(6, b); } 

void Cpu::opCb_0xf1() { opSet(6, c); } 

void Cpu::opCb_0xf2() { opSet(6, d); } 

void Cpu::opCb_0xf3() { opSet(6, e); } 

void Cpu::opCb_0xf4() { opSet(6, h); } 

void Cpu::opCb_0xf5() { opSet(6, l); } 

void Cpu::opCb_0xf6()
{
    unsigned short addr = getHL();
    opSet(6, addr); 
} 

void Cpu::opCb_0xf7() { opSet(6, a); } 

void Cpu::opCb_0xf8() { opSet(7, b); } 

void Cpu::opCb_0xf9() { opSet(7, c); } 

void Cpu::opCb_0xfa() { opSet(7, d); } 

void Cpu::opCb_0xfb() { opSet(7, e); } 

void Cpu::opCb_0xfc() { opSet(7, h); } 

void Cpu::opCb_0xfd() { opSet(7, l); } 

void Cpu::opCb_0xfe()
{
    unsigned short addr = getHL();
    opSet(7, addr); 
} 

void Cpu::opCb_0xff() { opSet(7, a); }

