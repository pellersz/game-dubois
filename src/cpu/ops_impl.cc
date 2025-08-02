#include "cpu.h"

/////////////////////////////////////////////////////////////////
// Helper instructions
/////////////////////////////////////////////////////////////////


// load 
void Cpu::ld(byte& dest, byte src) { dest = src; } 

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
    setZF(!new_val); 
    setHF((A & 0b111111111111u) < (tmp & 0b111111111111u));
    setNF(false);
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
// TODO: like above
//void Cpu::opDec(std::function<word()> get, std::function<void(word)> set) { set(get() - 1); }


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

void Cpu::opCpl(byte)
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

void Cpu::opRrca(byte&)
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
    
}   

void Cpu::opJp(word addr)
{

}

void Cpu::opJr(word addr)
{

}

void Cpu::opRet()
{
    
}

void Cpu::opReti()
{

}

void Cpu::opRst()
{
    
}

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

void Cpu::opPush(word reg)
{
    memory.writeWord(SP, reg);
    stack2Step();
}

void Cpu::opDi() { IME = false; }

void Cpu::opEi() { IME = true; }

// TODO: do this after handler is done
void Cpu::opHalt()
{
    if (IME) 
    {
        
    }
    else if (!(getIE() & getIF()))
    {

    }
    else 
    {

    }
}

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


void Cpu::op_0x00() { opNop(); ++PC; } 

void Cpu::op_0x01() {  ++PC; } 

void Cpu::op_0x02() {  ++PC; } 

void Cpu::op_0x03() {  ++PC; } 

void Cpu::op_0x04() {  ++PC; } 

void Cpu::op_0x05() {  ++PC; } 

void Cpu::op_0x06() {  ++PC; } 

void Cpu::op_0x07() {  ++PC; } 

void Cpu::op_0x08() {  ++PC; } 

void Cpu::op_0x09() {  ++PC; } 

void Cpu::op_0x0a() {  ++PC; } 

void Cpu::op_0x0b() {  ++PC; } 

void Cpu::op_0x0c() {  ++PC; } 

void Cpu::op_0x0d() {  ++PC; } 

void Cpu::op_0x0e() {  ++PC; } 

void Cpu::op_0x0f() {  ++PC; } 

void Cpu::op_0x10() {  ++PC; } 

void Cpu::op_0x11() {  ++PC; } 

void Cpu::op_0x12() {  ++PC; } 

void Cpu::op_0x13() {  ++PC; } 

void Cpu::op_0x14() {  ++PC; } 

void Cpu::op_0x15() {  ++PC; } 

void Cpu::op_0x16() {  ++PC; } 

void Cpu::op_0x17() {  ++PC; } 

void Cpu::op_0x18() {  ++PC; } 

void Cpu::op_0x19() {  ++PC; } 

void Cpu::op_0x1a() {  ++PC; } 

void Cpu::op_0x1b() {  ++PC; } 

void Cpu::op_0x1c() {  ++PC; } 

void Cpu::op_0x1d() {  ++PC; } 

void Cpu::op_0x1e() {  ++PC; } 

void Cpu::op_0x1f() {  ++PC; } 

void Cpu::op_0x20() {  ++PC; } 

void Cpu::op_0x21() {  ++PC; } 

void Cpu::op_0x22() {  ++PC; } 

void Cpu::op_0x23() {  ++PC; } 

void Cpu::op_0x24() {  ++PC; } 

void Cpu::op_0x25() {  ++PC; } 

void Cpu::op_0x26() {  ++PC; } 

void Cpu::op_0x27() {  ++PC; } 

void Cpu::op_0x28() {  ++PC; } 

void Cpu::op_0x29() {  ++PC; } 

void Cpu::op_0x2a() {  ++PC; } 

void Cpu::op_0x2b() {  ++PC; } 

void Cpu::op_0x2c() {  ++PC; } 

void Cpu::op_0x2d() {  ++PC; } 

void Cpu::op_0x2e() {  ++PC; } 

void Cpu::op_0x2f() {  ++PC; } 

void Cpu::op_0x30() {  ++PC; } 

void Cpu::op_0x31() {  ++PC; } 

void Cpu::op_0x32() {  ++PC; } 

void Cpu::op_0x33() {  ++PC; } 

void Cpu::op_0x34() {  ++PC; } 

void Cpu::op_0x35() {  ++PC; } 

void Cpu::op_0x36() {  ++PC; } 

void Cpu::op_0x37() {  ++PC; } 

void Cpu::op_0x38() {  ++PC; } 

void Cpu::op_0x39() {  ++PC; } 

void Cpu::op_0x3a() {  ++PC; } 

void Cpu::op_0x3b() {  ++PC; } 

void Cpu::op_0x3c() {  ++PC; } 

void Cpu::op_0x3d() {  ++PC; } 

void Cpu::op_0x3e() {  ++PC; } 

void Cpu::op_0x3f() {  ++PC; } 

void Cpu::op_0x40() {  ++PC; } 

void Cpu::op_0x41() {  ++PC; } 

void Cpu::op_0x42() {  ++PC; } 

void Cpu::op_0x43() {  ++PC; } 

void Cpu::op_0x44() {  ++PC; } 

void Cpu::op_0x45() {  ++PC; } 

void Cpu::op_0x46() {  ++PC; } 

void Cpu::op_0x47() {  ++PC; } 

void Cpu::op_0x48() {  ++PC; } 

void Cpu::op_0x49() {  ++PC; } 

void Cpu::op_0x4a() {  ++PC; } 

void Cpu::op_0x4b() {  ++PC; } 

void Cpu::op_0x4c() {  ++PC; } 

void Cpu::op_0x4d() {  ++PC; } 

void Cpu::op_0x4e() {  ++PC; } 

void Cpu::op_0x4f() {  ++PC; } 

void Cpu::op_0x50() {  ++PC; } 

void Cpu::op_0x51() {  ++PC; } 

void Cpu::op_0x52() {  ++PC; } 

void Cpu::op_0x53() {  ++PC; } 

void Cpu::op_0x54() {  ++PC; } 

void Cpu::op_0x55() {  ++PC; } 

void Cpu::op_0x56() {  ++PC; } 

void Cpu::op_0x57() {  ++PC; } 

void Cpu::op_0x58() {  ++PC; } 

void Cpu::op_0x59() {  ++PC; } 

void Cpu::op_0x5a() {  ++PC; } 

void Cpu::op_0x5b() {  ++PC; } 

void Cpu::op_0x5c() {  ++PC; } 

void Cpu::op_0x5d() {  ++PC; } 

void Cpu::op_0x5e() {  ++PC; } 

void Cpu::op_0x5f() {  ++PC; } 

void Cpu::op_0x60() {  ++PC; } 

void Cpu::op_0x61() {  ++PC; } 

void Cpu::op_0x62() {  ++PC; } 

void Cpu::op_0x63() {  ++PC; } 

void Cpu::op_0x64() {  ++PC; } 

void Cpu::op_0x65() {  ++PC; } 

void Cpu::op_0x66() {  ++PC; } 

void Cpu::op_0x67() {  ++PC; } 

void Cpu::op_0x68() {  ++PC; } 

void Cpu::op_0x69() {  ++PC; } 

void Cpu::op_0x6a() {  ++PC; } 

void Cpu::op_0x6b() {  ++PC; } 

void Cpu::op_0x6c() {  ++PC; } 

void Cpu::op_0x6d() {  ++PC; } 

void Cpu::op_0x6e() {  ++PC; } 

void Cpu::op_0x6f() {  ++PC; } 

void Cpu::op_0x70() {  ++PC; } 

void Cpu::op_0x71() {  ++PC; } 

void Cpu::op_0x72() {  ++PC; } 

void Cpu::op_0x73() {  ++PC; } 

void Cpu::op_0x74() {  ++PC; } 

void Cpu::op_0x75() {  ++PC; } 

void Cpu::op_0x76() {  ++PC; } 

void Cpu::op_0x77() {  ++PC; } 

void Cpu::op_0x78() {  ++PC; } 

void Cpu::op_0x79() {  ++PC; } 

void Cpu::op_0x7a() {  ++PC; } 

void Cpu::op_0x7b() {  ++PC; } 

void Cpu::op_0x7c() {  ++PC; } 

void Cpu::op_0x7d() {  ++PC; } 

void Cpu::op_0x7e() {  ++PC; } 

void Cpu::op_0x7f() {  ++PC; } 

void Cpu::op_0x80() {  ++PC; } 

void Cpu::op_0x81() {  ++PC; } 

void Cpu::op_0x82() {  ++PC; } 

void Cpu::op_0x83() {  ++PC; } 

void Cpu::op_0x84() {  ++PC; } 

void Cpu::op_0x85() {  ++PC; } 

void Cpu::op_0x86() {  ++PC; } 

void Cpu::op_0x87() {  ++PC; } 

void Cpu::op_0x88() {  ++PC; } 

void Cpu::op_0x89() {  ++PC; } 

void Cpu::op_0x8a() {  ++PC; } 

void Cpu::op_0x8b() {  ++PC; } 

void Cpu::op_0x8c() {  ++PC; } 

void Cpu::op_0x8d() {  ++PC; } 

void Cpu::op_0x8e() {  ++PC; } 

void Cpu::op_0x8f() {  ++PC; } 

void Cpu::op_0x90() {  ++PC; } 

void Cpu::op_0x91() {  ++PC; } 

void Cpu::op_0x92() {  ++PC; } 

void Cpu::op_0x93() {  ++PC; } 

void Cpu::op_0x94() {  ++PC; } 

void Cpu::op_0x95() {  ++PC; } 

void Cpu::op_0x96() {  ++PC; } 

void Cpu::op_0x97() {  ++PC; } 

void Cpu::op_0x98() {  ++PC; } 

void Cpu::op_0x99() {  ++PC; } 

void Cpu::op_0x9a() {  ++PC; } 

void Cpu::op_0x9b() {  ++PC; } 

void Cpu::op_0x9c() {  ++PC; } 

void Cpu::op_0x9d() {  ++PC; } 

void Cpu::op_0x9e() {  ++PC; } 

void Cpu::op_0x9f() {  ++PC; } 

void Cpu::op_0xa0() {  ++PC; } 

void Cpu::op_0xa1() {  ++PC; } 

void Cpu::op_0xa2() {  ++PC; } 

void Cpu::op_0xa3() {  ++PC; } 

void Cpu::op_0xa4() {  ++PC; } 

void Cpu::op_0xa5() {  ++PC; } 

void Cpu::op_0xa6() {  ++PC; } 

void Cpu::op_0xa7() {  ++PC; } 

void Cpu::op_0xa8() {  ++PC; } 

void Cpu::op_0xa9() {  ++PC; } 

void Cpu::op_0xaa() {  ++PC; } 

void Cpu::op_0xab() {  ++PC; } 

void Cpu::op_0xac() {  ++PC; } 

void Cpu::op_0xad() {  ++PC; } 

void Cpu::op_0xae() {  ++PC; } 

void Cpu::op_0xaf() {  ++PC; } 

void Cpu::op_0xb0() {  ++PC; } 

void Cpu::op_0xb1() {  ++PC; } 

void Cpu::op_0xb2() {  ++PC; } 

void Cpu::op_0xb3() {  ++PC; } 

void Cpu::op_0xb4() {  ++PC; } 

void Cpu::op_0xb5() {  ++PC; } 

void Cpu::op_0xb6() {  ++PC; } 

void Cpu::op_0xb7() {  ++PC; } 

void Cpu::op_0xb8() {  ++PC; } 

void Cpu::op_0xb9() {  ++PC; } 

void Cpu::op_0xba() {  ++PC; } 

void Cpu::op_0xbb() {  ++PC; } 

void Cpu::op_0xbc() {  ++PC; } 

void Cpu::op_0xbd() {  ++PC; } 

void Cpu::op_0xbe() {  ++PC; } 

void Cpu::op_0xbf() {  ++PC; } 

void Cpu::op_0xc0() {  ++PC; } 

void Cpu::op_0xc1() {  ++PC; } 

void Cpu::op_0xc2() {  ++PC; } 

void Cpu::op_0xc3() {  ++PC; } 

void Cpu::op_0xc4() {  ++PC; } 

void Cpu::op_0xc5() {  ++PC; } 

void Cpu::op_0xc6() {  ++PC; } 

void Cpu::op_0xc7() {  ++PC; } 

void Cpu::op_0xc8() {  ++PC; } 

void Cpu::op_0xc9() {  ++PC; } 

void Cpu::op_0xca() {  ++PC; } 

void Cpu::op_0xcb() {  ++PC; } 

void Cpu::op_0xcc() {  ++PC; } 

void Cpu::op_0xcd() {  ++PC; } 

void Cpu::op_0xce() {  ++PC; } 

void Cpu::op_0xcf() {  ++PC; } 

void Cpu::op_0xd0() {  ++PC; } 

void Cpu::op_0xd1() {  ++PC; } 

void Cpu::op_0xd2() {  ++PC; } 

void Cpu::op_0xd3() {  ++PC; } 

void Cpu::op_0xd4() {  ++PC; } 

void Cpu::op_0xd5() {  ++PC; } 

void Cpu::op_0xd6() {  ++PC; } 

void Cpu::op_0xd7() {  ++PC; } 

void Cpu::op_0xd8() {  ++PC; } 

void Cpu::op_0xd9() {  ++PC; } 

void Cpu::op_0xda() {  ++PC; } 

void Cpu::op_0xdb() {  ++PC; } 

void Cpu::op_0xdc() {  ++PC; } 

void Cpu::op_0xdd() {  ++PC; } 

void Cpu::op_0xde() {  ++PC; } 

void Cpu::op_0xdf() {  ++PC; } 

void Cpu::op_0xe0() {  ++PC; } 

void Cpu::op_0xe1() {  ++PC; } 

void Cpu::op_0xe2() {  ++PC; } 

void Cpu::op_0xe3() {  ++PC; } 

void Cpu::op_0xe4() {  ++PC; } 

void Cpu::op_0xe5() {  ++PC; } 

void Cpu::op_0xe6() {  ++PC; } 

void Cpu::op_0xe7() {  ++PC; } 

void Cpu::op_0xe8() {  ++PC; } 

void Cpu::op_0xe9() {  ++PC; } 

void Cpu::op_0xea() {  ++PC; } 

void Cpu::op_0xeb() {  ++PC; } 

void Cpu::op_0xec() {  ++PC; } 

void Cpu::op_0xed() {  ++PC; } 

void Cpu::op_0xee() {  ++PC; } 

void Cpu::op_0xef() {  ++PC; } 

void Cpu::op_0xf0() {  ++PC; } 

void Cpu::op_0xf1() {  ++PC; } 

void Cpu::op_0xf2() {  ++PC; } 

void Cpu::op_0xf3() {  ++PC; } 

void Cpu::op_0xf4() {  ++PC; } 

void Cpu::op_0xf5() {  ++PC; } 

void Cpu::op_0xf6() {  ++PC; } 

void Cpu::op_0xf7() {  ++PC; } 

void Cpu::op_0xf8() {  ++PC; } 

void Cpu::op_0xf9() {  ++PC; } 

void Cpu::op_0xfa() {  ++PC; } 

void Cpu::op_0xfb() {  ++PC; } 

void Cpu::op_0xfc() {  ++PC; } 

void Cpu::op_0xfd() {  ++PC; } 

void Cpu::op_0xfe() {  ++PC; } 

void Cpu::op_0xff() {  ++PC; } 


/////////////////////////////////////////////////////////////////
// bc (16 bit) operations
/////////////////////////////////////////////////////////////////


void Cpu::opCb_0x00() {  ++PC; } 

void Cpu::opCb_0x01() {  ++PC; } 

void Cpu::opCb_0x02() {  ++PC; } 

void Cpu::opCb_0x03() {  ++PC; } 

void Cpu::opCb_0x04() {  ++PC; } 

void Cpu::opCb_0x05() {  ++PC; } 

void Cpu::opCb_0x06() {  ++PC; } 

void Cpu::opCb_0x07() {  ++PC; } 

void Cpu::opCb_0x08() {  ++PC; } 

void Cpu::opCb_0x09() {  ++PC; } 

void Cpu::opCb_0x0a() {  ++PC; } 

void Cpu::opCb_0x0b() {  ++PC; } 

void Cpu::opCb_0x0c() {  ++PC; } 

void Cpu::opCb_0x0d() {  ++PC; } 

void Cpu::opCb_0x0e() {  ++PC; } 

void Cpu::opCb_0x0f() {  ++PC; } 

void Cpu::opCb_0x10() {  ++PC; } 

void Cpu::opCb_0x11() {  ++PC; } 

void Cpu::opCb_0x12() {  ++PC; } 

void Cpu::opCb_0x13() {  ++PC; } 

void Cpu::opCb_0x14() {  ++PC; } 

void Cpu::opCb_0x15() {  ++PC; } 

void Cpu::opCb_0x16() {  ++PC; } 

void Cpu::opCb_0x17() {  ++PC; } 

void Cpu::opCb_0x18() {  ++PC; } 

void Cpu::opCb_0x19() {  ++PC; } 

void Cpu::opCb_0x1a() {  ++PC; } 

void Cpu::opCb_0x1b() {  ++PC; } 

void Cpu::opCb_0x1c() {  ++PC; } 

void Cpu::opCb_0x1d() {  ++PC; } 

void Cpu::opCb_0x1e() {  ++PC; } 

void Cpu::opCb_0x1f() {  ++PC; } 

void Cpu::opCb_0x20() {  ++PC; } 

void Cpu::opCb_0x21() {  ++PC; } 

void Cpu::opCb_0x22() {  ++PC; } 

void Cpu::opCb_0x23() {  ++PC; } 

void Cpu::opCb_0x24() {  ++PC; } 

void Cpu::opCb_0x25() {  ++PC; } 

void Cpu::opCb_0x26() {  ++PC; } 

void Cpu::opCb_0x27() {  ++PC; } 

void Cpu::opCb_0x28() {  ++PC; } 

void Cpu::opCb_0x29() {  ++PC; } 

void Cpu::opCb_0x2a() {  ++PC; } 

void Cpu::opCb_0x2b() {  ++PC; } 

void Cpu::opCb_0x2c() {  ++PC; } 

void Cpu::opCb_0x2d() {  ++PC; } 

void Cpu::opCb_0x2e() {  ++PC; } 

void Cpu::opCb_0x2f() {  ++PC; } 

void Cpu::opCb_0x30() {  ++PC; } 

void Cpu::opCb_0x31() {  ++PC; } 

void Cpu::opCb_0x32() {  ++PC; } 

void Cpu::opCb_0x33() {  ++PC; } 

void Cpu::opCb_0x34() {  ++PC; } 

void Cpu::opCb_0x35() {  ++PC; } 

void Cpu::opCb_0x36() {  ++PC; } 

void Cpu::opCb_0x37() {  ++PC; } 

void Cpu::opCb_0x38() {  ++PC; } 

void Cpu::opCb_0x39() {  ++PC; } 

void Cpu::opCb_0x3a() {  ++PC; } 

void Cpu::opCb_0x3b() {  ++PC; } 

void Cpu::opCb_0x3c() {  ++PC; } 

void Cpu::opCb_0x3d() {  ++PC; } 

void Cpu::opCb_0x3e() {  ++PC; } 

void Cpu::opCb_0x3f() {  ++PC; } 

void Cpu::opCb_0x40() {  ++PC; } 

void Cpu::opCb_0x41() {  ++PC; } 

void Cpu::opCb_0x42() {  ++PC; } 

void Cpu::opCb_0x43() {  ++PC; } 

void Cpu::opCb_0x44() {  ++PC; } 

void Cpu::opCb_0x45() {  ++PC; } 

void Cpu::opCb_0x46() {  ++PC; } 

void Cpu::opCb_0x47() {  ++PC; } 

void Cpu::opCb_0x48() {  ++PC; } 

void Cpu::opCb_0x49() {  ++PC; } 

void Cpu::opCb_0x4a() {  ++PC; } 

void Cpu::opCb_0x4b() {  ++PC; } 

void Cpu::opCb_0x4c() {  ++PC; } 

void Cpu::opCb_0x4d() {  ++PC; } 

void Cpu::opCb_0x4e() {  ++PC; } 

void Cpu::opCb_0x4f() {  ++PC; } 

void Cpu::opCb_0x50() {  ++PC; } 

void Cpu::opCb_0x51() {  ++PC; } 

void Cpu::opCb_0x52() {  ++PC; } 

void Cpu::opCb_0x53() {  ++PC; } 

void Cpu::opCb_0x54() {  ++PC; } 

void Cpu::opCb_0x55() {  ++PC; } 

void Cpu::opCb_0x56() {  ++PC; } 

void Cpu::opCb_0x57() {  ++PC; } 

void Cpu::opCb_0x58() {  ++PC; } 

void Cpu::opCb_0x59() {  ++PC; } 

void Cpu::opCb_0x5a() {  ++PC; } 

void Cpu::opCb_0x5b() {  ++PC; } 

void Cpu::opCb_0x5c() {  ++PC; } 

void Cpu::opCb_0x5d() {  ++PC; } 

void Cpu::opCb_0x5e() {  ++PC; } 

void Cpu::opCb_0x5f() {  ++PC; } 

void Cpu::opCb_0x60() {  ++PC; } 

void Cpu::opCb_0x61() {  ++PC; } 

void Cpu::opCb_0x62() {  ++PC; } 

void Cpu::opCb_0x63() {  ++PC; } 

void Cpu::opCb_0x64() {  ++PC; } 

void Cpu::opCb_0x65() {  ++PC; } 

void Cpu::opCb_0x66() {  ++PC; } 

void Cpu::opCb_0x67() {  ++PC; } 

void Cpu::opCb_0x68() {  ++PC; } 

void Cpu::opCb_0x69() {  ++PC; } 

void Cpu::opCb_0x6a() {  ++PC; } 

void Cpu::opCb_0x6b() {  ++PC; } 

void Cpu::opCb_0x6c() {  ++PC; } 

void Cpu::opCb_0x6d() {  ++PC; } 

void Cpu::opCb_0x6e() {  ++PC; } 

void Cpu::opCb_0x6f() {  ++PC; } 

void Cpu::opCb_0x70() {  ++PC; } 

void Cpu::opCb_0x71() {  ++PC; } 

void Cpu::opCb_0x72() {  ++PC; } 

void Cpu::opCb_0x73() {  ++PC; } 

void Cpu::opCb_0x74() {  ++PC; } 

void Cpu::opCb_0x75() {  ++PC; } 

void Cpu::opCb_0x76() {  ++PC; } 

void Cpu::opCb_0x77() {  ++PC; } 

void Cpu::opCb_0x78() {  ++PC; } 

void Cpu::opCb_0x79() {  ++PC; } 

void Cpu::opCb_0x7a() {  ++PC; } 

void Cpu::opCb_0x7b() {  ++PC; } 

void Cpu::opCb_0x7c() {  ++PC; } 

void Cpu::opCb_0x7d() {  ++PC; } 

void Cpu::opCb_0x7e() {  ++PC; } 

void Cpu::opCb_0x7f() {  ++PC; } 

void Cpu::opCb_0x80() {  ++PC; } 

void Cpu::opCb_0x81() {  ++PC; } 

void Cpu::opCb_0x82() {  ++PC; } 

void Cpu::opCb_0x83() {  ++PC; } 

void Cpu::opCb_0x84() {  ++PC; } 

void Cpu::opCb_0x85() {  ++PC; } 

void Cpu::opCb_0x86() {  ++PC; } 

void Cpu::opCb_0x87() {  ++PC; } 

void Cpu::opCb_0x88() {  ++PC; } 

void Cpu::opCb_0x89() {  ++PC; } 

void Cpu::opCb_0x8a() {  ++PC; } 

void Cpu::opCb_0x8b() {  ++PC; } 

void Cpu::opCb_0x8c() {  ++PC; } 

void Cpu::opCb_0x8d() {  ++PC; } 

void Cpu::opCb_0x8e() {  ++PC; } 

void Cpu::opCb_0x8f() {  ++PC; } 

void Cpu::opCb_0x90() {  ++PC; } 

void Cpu::opCb_0x91() {  ++PC; } 

void Cpu::opCb_0x92() {  ++PC; } 

void Cpu::opCb_0x93() {  ++PC; } 

void Cpu::opCb_0x94() {  ++PC; } 

void Cpu::opCb_0x95() {  ++PC; } 

void Cpu::opCb_0x96() {  ++PC; } 

void Cpu::opCb_0x97() {  ++PC; } 

void Cpu::opCb_0x98() {  ++PC; } 

void Cpu::opCb_0x99() {  ++PC; } 

void Cpu::opCb_0x9a() {  ++PC; } 

void Cpu::opCb_0x9b() {  ++PC; } 

void Cpu::opCb_0x9c() {  ++PC; } 

void Cpu::opCb_0x9d() {  ++PC; } 

void Cpu::opCb_0x9e() {  ++PC; } 

void Cpu::opCb_0x9f() {  ++PC; } 

void Cpu::opCb_0xa0() {  ++PC; } 

void Cpu::opCb_0xa1() {  ++PC; } 

void Cpu::opCb_0xa2() {  ++PC; } 

void Cpu::opCb_0xa3() {  ++PC; } 

void Cpu::opCb_0xa4() {  ++PC; } 

void Cpu::opCb_0xa5() {  ++PC; } 

void Cpu::opCb_0xa6() {  ++PC; } 

void Cpu::opCb_0xa7() {  ++PC; } 

void Cpu::opCb_0xa8() {  ++PC; } 

void Cpu::opCb_0xa9() {  ++PC; } 

void Cpu::opCb_0xaa() {  ++PC; } 

void Cpu::opCb_0xab() {  ++PC; } 

void Cpu::opCb_0xac() {  ++PC; } 

void Cpu::opCb_0xad() {  ++PC; } 

void Cpu::opCb_0xae() {  ++PC; } 

void Cpu::opCb_0xaf() {  ++PC; } 

void Cpu::opCb_0xb0() {  ++PC; } 

void Cpu::opCb_0xb1() {  ++PC; } 

void Cpu::opCb_0xb2() {  ++PC; } 

void Cpu::opCb_0xb3() {  ++PC; } 

void Cpu::opCb_0xb4() {  ++PC; } 

void Cpu::opCb_0xb5() {  ++PC; } 

void Cpu::opCb_0xb6() {  ++PC; } 

void Cpu::opCb_0xb7() {  ++PC; } 

void Cpu::opCb_0xb8() {  ++PC; } 

void Cpu::opCb_0xb9() {  ++PC; } 

void Cpu::opCb_0xba() {  ++PC; } 

void Cpu::opCb_0xbb() {  ++PC; } 

void Cpu::opCb_0xbc() {  ++PC; } 

void Cpu::opCb_0xbd() {  ++PC; } 

void Cpu::opCb_0xbe() {  ++PC; } 

void Cpu::opCb_0xbf() {  ++PC; } 

void Cpu::opCb_0xc0() {  ++PC; } 

void Cpu::opCb_0xc1() {  ++PC; } 

void Cpu::opCb_0xc2() {  ++PC; } 

void Cpu::opCb_0xc3() {  ++PC; } 

void Cpu::opCb_0xc4() {  ++PC; } 

void Cpu::opCb_0xc5() {  ++PC; } 

void Cpu::opCb_0xc6() {  ++PC; } 

void Cpu::opCb_0xc7() {  ++PC; } 

void Cpu::opCb_0xc8() {  ++PC; } 

void Cpu::opCb_0xc9() {  ++PC; } 

void Cpu::opCb_0xca() {  ++PC; } 

void Cpu::opCb_0xcb() {  ++PC; } 

void Cpu::opCb_0xcc() {  ++PC; } 

void Cpu::opCb_0xcd() {  ++PC; } 

void Cpu::opCb_0xce() {  ++PC; } 

void Cpu::opCb_0xcf() {  ++PC; } 

void Cpu::opCb_0xd0() {  ++PC; } 

void Cpu::opCb_0xd1() {  ++PC; } 

void Cpu::opCb_0xd2() {  ++PC; } 

void Cpu::opCb_0xd3() {  ++PC; } 

void Cpu::opCb_0xd4() {  ++PC; } 

void Cpu::opCb_0xd5() {  ++PC; } 

void Cpu::opCb_0xd6() {  ++PC; } 

void Cpu::opCb_0xd7() {  ++PC; } 

void Cpu::opCb_0xd8() {  ++PC; } 

void Cpu::opCb_0xd9() {  ++PC; } 

void Cpu::opCb_0xda() {  ++PC; } 

void Cpu::opCb_0xdb() {  ++PC; } 

void Cpu::opCb_0xdc() {  ++PC; } 

void Cpu::opCb_0xdd() {  ++PC; } 

void Cpu::opCb_0xde() {  ++PC; } 

void Cpu::opCb_0xdf() {  ++PC; } 

void Cpu::opCb_0xe0() {  ++PC; } 

void Cpu::opCb_0xe1() {  ++PC; } 

void Cpu::opCb_0xe2() {  ++PC; } 

void Cpu::opCb_0xe3() {  ++PC; } 

void Cpu::opCb_0xe4() {  ++PC; } 

void Cpu::opCb_0xe5() {  ++PC; } 

void Cpu::opCb_0xe6() {  ++PC; } 

void Cpu::opCb_0xe7() {  ++PC; } 

void Cpu::opCb_0xe8() {  ++PC; } 

void Cpu::opCb_0xe9() {  ++PC; } 

void Cpu::opCb_0xea() {  ++PC; } 

void Cpu::opCb_0xeb() {  ++PC; } 

void Cpu::opCb_0xec() {  ++PC; } 

void Cpu::opCb_0xed() {  ++PC; } 

void Cpu::opCb_0xee() {  ++PC; } 

void Cpu::opCb_0xef() {  ++PC; } 

void Cpu::opCb_0xf0() {  ++PC; } 

void Cpu::opCb_0xf1() {  ++PC; } 

void Cpu::opCb_0xf2() {  ++PC; } 

void Cpu::opCb_0xf3() {  ++PC; } 

void Cpu::opCb_0xf4() {  ++PC; } 

void Cpu::opCb_0xf5() {  ++PC; } 

void Cpu::opCb_0xf6() {  ++PC; } 

void Cpu::opCb_0xf7() {  ++PC; } 

void Cpu::opCb_0xf8() {  ++PC; } 

void Cpu::opCb_0xf9() {  ++PC; } 

void Cpu::opCb_0xfa() {  ++PC; } 

void Cpu::opCb_0xfb() {  ++PC; } 

void Cpu::opCb_0xfc() {  ++PC; } 

void Cpu::opCb_0xfd() {  ++PC; } 

void Cpu::opCb_0xfe() {  ++PC; } 

void Cpu::opCb_0xff() {  ++PC; }
