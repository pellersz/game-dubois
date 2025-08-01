#include "cpu.h"
#include "cpu.cc"

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
    // TODO: see if this actually does what you want it to
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

// TODO: decide what to do with this
//void Cpu::opInc(std::function<word()> get, std::function<void(word)> set) { set(get() + 1); }


void Cpu::opDec(byte& dest) {
    byte tmp = dest--;
    dest -= 1; 
    setZF(!dest); 
    setHF((dest & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}

void Cpu::opDec(word& dest) { ++dest; }

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

void Cpu::opAdd(char val)
{
    word tmp = SP;
    SP += val;
    F &= HF_MASK | CF_MASK;
    setCF((SP & 0b11111111) < (tmp & 0b11111111)); 
    setHF((SP & 0b1111u) < (tmp & 0b1111u));
}

// TODO: do this after memory management is done
void Cpu::opPop(word& reg)
{
    
}

// TODO: do this after memory management is done
void Cpu::opPush(word reg)
{
    
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


void Cpu::op_0x00() {} 

void Cpu::op_0x01() {} 

void Cpu::op_0x02() {} 

void Cpu::op_0x03() {} 

void Cpu::op_0x04() {} 

void Cpu::op_0x05() {} 

void Cpu::op_0x06() {} 

void Cpu::op_0x07() {} 

void Cpu::op_0x08() {} 

void Cpu::op_0x09() {} 

void Cpu::op_0x0a() {} 

void Cpu::op_0x0b() {} 

void Cpu::op_0x0c() {} 

void Cpu::op_0x0d() {} 

void Cpu::op_0x0e() {} 

void Cpu::op_0x0f() {} 

void Cpu::op_0x10() {} 

void Cpu::op_0x11() {} 

void Cpu::op_0x12() {} 

void Cpu::op_0x13() {} 

void Cpu::op_0x14() {} 

void Cpu::op_0x15() {} 

void Cpu::op_0x16() {} 

void Cpu::op_0x17() {} 

void Cpu::op_0x18() {} 

void Cpu::op_0x19() {} 

void Cpu::op_0x1a() {} 

void Cpu::op_0x1b() {} 

void Cpu::op_0x1c() {} 

void Cpu::op_0x1d() {} 

void Cpu::op_0x1e() {} 

void Cpu::op_0x1f() {} 

void Cpu::op_0x20() {} 

void Cpu::op_0x21() {} 

void Cpu::op_0x22() {} 

void Cpu::op_0x23() {} 

void Cpu::op_0x24() {} 

void Cpu::op_0x25() {} 

void Cpu::op_0x26() {} 

void Cpu::op_0x27() {} 

void Cpu::op_0x28() {} 

void Cpu::op_0x29() {} 

void Cpu::op_0x2a() {} 

void Cpu::op_0x2b() {} 

void Cpu::op_0x2c() {} 

void Cpu::op_0x2d() {} 

void Cpu::op_0x2e() {} 

void Cpu::op_0x2f() {} 

void Cpu::op_0x30() {} 

void Cpu::op_0x31() {} 

void Cpu::op_0x32() {} 

void Cpu::op_0x33() {} 

void Cpu::op_0x34() {} 

void Cpu::op_0x35() {} 

void Cpu::op_0x36() {} 

void Cpu::op_0x37() {} 

void Cpu::op_0x38() {} 

void Cpu::op_0x39() {} 

void Cpu::op_0x3a() {} 

void Cpu::op_0x3b() {} 

void Cpu::op_0x3c() {} 

void Cpu::op_0x3d() {} 

void Cpu::op_0x3e() {} 

void Cpu::op_0x3f() {} 

void Cpu::op_0x40() {} 

void Cpu::op_0x41() {} 

void Cpu::op_0x42() {} 

void Cpu::op_0x43() {} 

void Cpu::op_0x44() {} 

void Cpu::op_0x45() {} 

void Cpu::op_0x46() {} 

void Cpu::op_0x47() {} 

void Cpu::op_0x48() {} 

void Cpu::op_0x49() {} 

void Cpu::op_0x4a() {} 

void Cpu::op_0x4b() {} 

void Cpu::op_0x4c() {} 

void Cpu::op_0x4d() {} 

void Cpu::op_0x4e() {} 

void Cpu::op_0x4f() {} 

void Cpu::op_0x50() {} 

void Cpu::op_0x51() {} 

void Cpu::op_0x52() {} 

void Cpu::op_0x53() {} 

void Cpu::op_0x54() {} 

void Cpu::op_0x55() {} 

void Cpu::op_0x56() {} 

void Cpu::op_0x57() {} 

void Cpu::op_0x58() {} 

void Cpu::op_0x59() {} 

void Cpu::op_0x5a() {} 

void Cpu::op_0x5b() {} 

void Cpu::op_0x5c() {} 

void Cpu::op_0x5d() {} 

void Cpu::op_0x5e() {} 

void Cpu::op_0x5f() {} 

void Cpu::op_0x60() {} 

void Cpu::op_0x61() {} 

void Cpu::op_0x62() {} 

void Cpu::op_0x63() {} 

void Cpu::op_0x64() {} 

void Cpu::op_0x65() {} 

void Cpu::op_0x66() {} 

void Cpu::op_0x67() {} 

void Cpu::op_0x68() {} 

void Cpu::op_0x69() {} 

void Cpu::op_0x6a() {} 

void Cpu::op_0x6b() {} 

void Cpu::op_0x6c() {} 

void Cpu::op_0x6d() {} 

void Cpu::op_0x6e() {} 

void Cpu::op_0x6f() {} 

void Cpu::op_0x70() {} 

void Cpu::op_0x71() {} 

void Cpu::op_0x72() {} 

void Cpu::op_0x73() {} 

void Cpu::op_0x74() {} 

void Cpu::op_0x75() {} 

void Cpu::op_0x76() {} 

void Cpu::op_0x77() {} 

void Cpu::op_0x78() {} 

void Cpu::op_0x79() {} 

void Cpu::op_0x7a() {} 

void Cpu::op_0x7b() {} 

void Cpu::op_0x7c() {} 

void Cpu::op_0x7d() {} 

void Cpu::op_0x7e() {} 

void Cpu::op_0x7f() {} 

void Cpu::op_0x80() {} 

void Cpu::op_0x81() {} 

void Cpu::op_0x82() {} 

void Cpu::op_0x83() {} 

void Cpu::op_0x84() {} 

void Cpu::op_0x85() {} 

void Cpu::op_0x86() {} 

void Cpu::op_0x87() {} 

void Cpu::op_0x88() {} 

void Cpu::op_0x89() {} 

void Cpu::op_0x8a() {} 

void Cpu::op_0x8b() {} 

void Cpu::op_0x8c() {} 

void Cpu::op_0x8d() {} 

void Cpu::op_0x8e() {} 

void Cpu::op_0x8f() {} 

void Cpu::op_0x90() {} 

void Cpu::op_0x91() {} 

void Cpu::op_0x92() {} 

void Cpu::op_0x93() {} 

void Cpu::op_0x94() {} 

void Cpu::op_0x95() {} 

void Cpu::op_0x96() {} 

void Cpu::op_0x97() {} 

void Cpu::op_0x98() {} 

void Cpu::op_0x99() {} 

void Cpu::op_0x9a() {} 

void Cpu::op_0x9b() {} 

void Cpu::op_0x9c() {} 

void Cpu::op_0x9d() {} 

void Cpu::op_0x9e() {} 

void Cpu::op_0x9f() {} 

void Cpu::op_0xa0() {} 

void Cpu::op_0xa1() {} 

void Cpu::op_0xa2() {} 

void Cpu::op_0xa3() {} 

void Cpu::op_0xa4() {} 

void Cpu::op_0xa5() {} 

void Cpu::op_0xa6() {} 

void Cpu::op_0xa7() {} 

void Cpu::op_0xa8() {} 

void Cpu::op_0xa9() {} 

void Cpu::op_0xaa() {} 

void Cpu::op_0xab() {} 

void Cpu::op_0xac() {} 

void Cpu::op_0xad() {} 

void Cpu::op_0xae() {} 

void Cpu::op_0xaf() {} 

void Cpu::op_0xb0() {} 

void Cpu::op_0xb1() {} 

void Cpu::op_0xb2() {} 

void Cpu::op_0xb3() {} 

void Cpu::op_0xb4() {} 

void Cpu::op_0xb5() {} 

void Cpu::op_0xb6() {} 

void Cpu::op_0xb7() {} 

void Cpu::op_0xb8() {} 

void Cpu::op_0xb9() {} 

void Cpu::op_0xba() {} 

void Cpu::op_0xbb() {} 

void Cpu::op_0xbc() {} 

void Cpu::op_0xbd() {} 

void Cpu::op_0xbe() {} 

void Cpu::op_0xbf() {} 

void Cpu::op_0xc0() {} 

void Cpu::op_0xc1() {} 

void Cpu::op_0xc2() {} 

void Cpu::op_0xc3() {} 

void Cpu::op_0xc4() {} 

void Cpu::op_0xc5() {} 

void Cpu::op_0xc6() {} 

void Cpu::op_0xc7() {} 

void Cpu::op_0xc8() {} 

void Cpu::op_0xc9() {} 

void Cpu::op_0xca() {} 

void Cpu::op_0xcb() {} 

void Cpu::op_0xcc() {} 

void Cpu::op_0xcd() {} 

void Cpu::op_0xce() {} 

void Cpu::op_0xcf() {} 

void Cpu::op_0xd0() {} 

void Cpu::op_0xd1() {} 

void Cpu::op_0xd2() {} 

void Cpu::op_0xd3() {} 

void Cpu::op_0xd4() {} 

void Cpu::op_0xd5() {} 

void Cpu::op_0xd6() {} 

void Cpu::op_0xd7() {} 

void Cpu::op_0xd8() {} 

void Cpu::op_0xd9() {} 

void Cpu::op_0xda() {} 

void Cpu::op_0xdb() {} 

void Cpu::op_0xdc() {} 

void Cpu::op_0xdd() {} 

void Cpu::op_0xde() {} 

void Cpu::op_0xdf() {} 

void Cpu::op_0xe0() {} 

void Cpu::op_0xe1() {} 

void Cpu::op_0xe2() {} 

void Cpu::op_0xe3() {} 

void Cpu::op_0xe4() {} 

void Cpu::op_0xe5() {} 

void Cpu::op_0xe6() {} 

void Cpu::op_0xe7() {} 

void Cpu::op_0xe8() {} 

void Cpu::op_0xe9() {} 

void Cpu::op_0xea() {} 

void Cpu::op_0xeb() {} 

void Cpu::op_0xec() {} 

void Cpu::op_0xed() {} 

void Cpu::op_0xee() {} 

void Cpu::op_0xef() {} 

void Cpu::op_0xf0() {} 

void Cpu::op_0xf1() {} 

void Cpu::op_0xf2() {} 

void Cpu::op_0xf3() {} 

void Cpu::op_0xf4() {} 

void Cpu::op_0xf5() {} 

void Cpu::op_0xf6() {} 

void Cpu::op_0xf7() {} 

void Cpu::op_0xf8() {} 

void Cpu::op_0xf9() {} 

void Cpu::op_0xfa() {} 

void Cpu::op_0xfb() {} 

void Cpu::op_0xfc() {} 

void Cpu::op_0xfd() {} 

void Cpu::op_0xfe() {} 

void Cpu::op_0xff() {} 


/////////////////////////////////////////////////////////////////
// bc (16 bit) operations
/////////////////////////////////////////////////////////////////


void Cpu::opCb_0x00() {} 

void Cpu::opCb_0x01() {} 

void Cpu::opCb_0x02() {} 

void Cpu::opCb_0x03() {} 

void Cpu::opCb_0x04() {} 

void Cpu::opCb_0x05() {} 

void Cpu::opCb_0x06() {} 

void Cpu::opCb_0x07() {} 

void Cpu::opCb_0x08() {} 

void Cpu::opCb_0x09() {} 

void Cpu::opCb_0x0a() {} 

void Cpu::opCb_0x0b() {} 

void Cpu::opCb_0x0c() {} 

void Cpu::opCb_0x0d() {} 

void Cpu::opCb_0x0e() {} 

void Cpu::opCb_0x0f() {} 

void Cpu::opCb_0x10() {} 

void Cpu::opCb_0x11() {} 

void Cpu::opCb_0x12() {} 

void Cpu::opCb_0x13() {} 

void Cpu::opCb_0x14() {} 

void Cpu::opCb_0x15() {} 

void Cpu::opCb_0x16() {} 

void Cpu::opCb_0x17() {} 

void Cpu::opCb_0x18() {} 

void Cpu::opCb_0x19() {} 

void Cpu::opCb_0x1a() {} 

void Cpu::opCb_0x1b() {} 

void Cpu::opCb_0x1c() {} 

void Cpu::opCb_0x1d() {} 

void Cpu::opCb_0x1e() {} 

void Cpu::opCb_0x1f() {} 

void Cpu::opCb_0x20() {} 

void Cpu::opCb_0x21() {} 

void Cpu::opCb_0x22() {} 

void Cpu::opCb_0x23() {} 

void Cpu::opCb_0x24() {} 

void Cpu::opCb_0x25() {} 

void Cpu::opCb_0x26() {} 

void Cpu::opCb_0x27() {} 

void Cpu::opCb_0x28() {} 

void Cpu::opCb_0x29() {} 

void Cpu::opCb_0x2a() {} 

void Cpu::opCb_0x2b() {} 

void Cpu::opCb_0x2c() {} 

void Cpu::opCb_0x2d() {} 

void Cpu::opCb_0x2e() {} 

void Cpu::opCb_0x2f() {} 

void Cpu::opCb_0x30() {} 

void Cpu::opCb_0x31() {} 

void Cpu::opCb_0x32() {} 

void Cpu::opCb_0x33() {} 

void Cpu::opCb_0x34() {} 

void Cpu::opCb_0x35() {} 

void Cpu::opCb_0x36() {} 

void Cpu::opCb_0x37() {} 

void Cpu::opCb_0x38() {} 

void Cpu::opCb_0x39() {} 

void Cpu::opCb_0x3a() {} 

void Cpu::opCb_0x3b() {} 

void Cpu::opCb_0x3c() {} 

void Cpu::opCb_0x3d() {} 

void Cpu::opCb_0x3e() {} 

void Cpu::opCb_0x3f() {} 

void Cpu::opCb_0x40() {} 

void Cpu::opCb_0x41() {} 

void Cpu::opCb_0x42() {} 

void Cpu::opCb_0x43() {} 

void Cpu::opCb_0x44() {} 

void Cpu::opCb_0x45() {} 

void Cpu::opCb_0x46() {} 

void Cpu::opCb_0x47() {} 

void Cpu::opCb_0x48() {} 

void Cpu::opCb_0x49() {} 

void Cpu::opCb_0x4a() {} 

void Cpu::opCb_0x4b() {} 

void Cpu::opCb_0x4c() {} 

void Cpu::opCb_0x4d() {} 

void Cpu::opCb_0x4e() {} 

void Cpu::opCb_0x4f() {} 

void Cpu::opCb_0x50() {} 

void Cpu::opCb_0x51() {} 

void Cpu::opCb_0x52() {} 

void Cpu::opCb_0x53() {} 

void Cpu::opCb_0x54() {} 

void Cpu::opCb_0x55() {} 

void Cpu::opCb_0x56() {} 

void Cpu::opCb_0x57() {} 

void Cpu::opCb_0x58() {} 

void Cpu::opCb_0x59() {} 

void Cpu::opCb_0x5a() {} 

void Cpu::opCb_0x5b() {} 

void Cpu::opCb_0x5c() {} 

void Cpu::opCb_0x5d() {} 

void Cpu::opCb_0x5e() {} 

void Cpu::opCb_0x5f() {} 

void Cpu::opCb_0x60() {} 

void Cpu::opCb_0x61() {} 

void Cpu::opCb_0x62() {} 

void Cpu::opCb_0x63() {} 

void Cpu::opCb_0x64() {} 

void Cpu::opCb_0x65() {} 

void Cpu::opCb_0x66() {} 

void Cpu::opCb_0x67() {} 

void Cpu::opCb_0x68() {} 

void Cpu::opCb_0x69() {} 

void Cpu::opCb_0x6a() {} 

void Cpu::opCb_0x6b() {} 

void Cpu::opCb_0x6c() {} 

void Cpu::opCb_0x6d() {} 

void Cpu::opCb_0x6e() {} 

void Cpu::opCb_0x6f() {} 

void Cpu::opCb_0x70() {} 

void Cpu::opCb_0x71() {} 

void Cpu::opCb_0x72() {} 

void Cpu::opCb_0x73() {} 

void Cpu::opCb_0x74() {} 

void Cpu::opCb_0x75() {} 

void Cpu::opCb_0x76() {} 

void Cpu::opCb_0x77() {} 

void Cpu::opCb_0x78() {} 

void Cpu::opCb_0x79() {} 

void Cpu::opCb_0x7a() {} 

void Cpu::opCb_0x7b() {} 

void Cpu::opCb_0x7c() {} 

void Cpu::opCb_0x7d() {} 

void Cpu::opCb_0x7e() {} 

void Cpu::opCb_0x7f() {} 

void Cpu::opCb_0x80() {} 

void Cpu::opCb_0x81() {} 

void Cpu::opCb_0x82() {} 

void Cpu::opCb_0x83() {} 

void Cpu::opCb_0x84() {} 

void Cpu::opCb_0x85() {} 

void Cpu::opCb_0x86() {} 

void Cpu::opCb_0x87() {} 

void Cpu::opCb_0x88() {} 

void Cpu::opCb_0x89() {} 

void Cpu::opCb_0x8a() {} 

void Cpu::opCb_0x8b() {} 

void Cpu::opCb_0x8c() {} 

void Cpu::opCb_0x8d() {} 

void Cpu::opCb_0x8e() {} 

void Cpu::opCb_0x8f() {} 

void Cpu::opCb_0x90() {} 

void Cpu::opCb_0x91() {} 

void Cpu::opCb_0x92() {} 

void Cpu::opCb_0x93() {} 

void Cpu::opCb_0x94() {} 

void Cpu::opCb_0x95() {} 

void Cpu::opCb_0x96() {} 

void Cpu::opCb_0x97() {} 

void Cpu::opCb_0x98() {} 

void Cpu::opCb_0x99() {} 

void Cpu::opCb_0x9a() {} 

void Cpu::opCb_0x9b() {} 

void Cpu::opCb_0x9c() {} 

void Cpu::opCb_0x9d() {} 

void Cpu::opCb_0x9e() {} 

void Cpu::opCb_0x9f() {} 

void Cpu::opCb_0xa0() {} 

void Cpu::opCb_0xa1() {} 

void Cpu::opCb_0xa2() {} 

void Cpu::opCb_0xa3() {} 

void Cpu::opCb_0xa4() {} 

void Cpu::opCb_0xa5() {} 

void Cpu::opCb_0xa6() {} 

void Cpu::opCb_0xa7() {} 

void Cpu::opCb_0xa8() {} 

void Cpu::opCb_0xa9() {} 

void Cpu::opCb_0xaa() {} 

void Cpu::opCb_0xab() {} 

void Cpu::opCb_0xac() {} 

void Cpu::opCb_0xad() {} 

void Cpu::opCb_0xae() {} 

void Cpu::opCb_0xaf() {} 

void Cpu::opCb_0xb0() {} 

void Cpu::opCb_0xb1() {} 

void Cpu::opCb_0xb2() {} 

void Cpu::opCb_0xb3() {} 

void Cpu::opCb_0xb4() {} 

void Cpu::opCb_0xb5() {} 

void Cpu::opCb_0xb6() {} 

void Cpu::opCb_0xb7() {} 

void Cpu::opCb_0xb8() {} 

void Cpu::opCb_0xb9() {} 

void Cpu::opCb_0xba() {} 

void Cpu::opCb_0xbb() {} 

void Cpu::opCb_0xbc() {} 

void Cpu::opCb_0xbd() {} 

void Cpu::opCb_0xbe() {} 

void Cpu::opCb_0xbf() {} 

void Cpu::opCb_0xc0() {} 

void Cpu::opCb_0xc1() {} 

void Cpu::opCb_0xc2() {} 

void Cpu::opCb_0xc3() {} 

void Cpu::opCb_0xc4() {} 

void Cpu::opCb_0xc5() {} 

void Cpu::opCb_0xc6() {} 

void Cpu::opCb_0xc7() {} 

void Cpu::opCb_0xc8() {} 

void Cpu::opCb_0xc9() {} 

void Cpu::opCb_0xca() {} 

void Cpu::opCb_0xcb() {} 

void Cpu::opCb_0xcc() {} 

void Cpu::opCb_0xcd() {} 

void Cpu::opCb_0xce() {} 

void Cpu::opCb_0xcf() {} 

void Cpu::opCb_0xd0() {} 

void Cpu::opCb_0xd1() {} 

void Cpu::opCb_0xd2() {} 

void Cpu::opCb_0xd3() {} 

void Cpu::opCb_0xd4() {} 

void Cpu::opCb_0xd5() {} 

void Cpu::opCb_0xd6() {} 

void Cpu::opCb_0xd7() {} 

void Cpu::opCb_0xd8() {} 

void Cpu::opCb_0xd9() {} 

void Cpu::opCb_0xda() {} 

void Cpu::opCb_0xdb() {} 

void Cpu::opCb_0xdc() {} 

void Cpu::opCb_0xdd() {} 

void Cpu::opCb_0xde() {} 

void Cpu::opCb_0xdf() {} 

void Cpu::opCb_0xe0() {} 

void Cpu::opCb_0xe1() {} 

void Cpu::opCb_0xe2() {} 

void Cpu::opCb_0xe3() {} 

void Cpu::opCb_0xe4() {} 

void Cpu::opCb_0xe5() {} 

void Cpu::opCb_0xe6() {} 

void Cpu::opCb_0xe7() {} 

void Cpu::opCb_0xe8() {} 

void Cpu::opCb_0xe9() {} 

void Cpu::opCb_0xea() {} 

void Cpu::opCb_0xeb() {} 

void Cpu::opCb_0xec() {} 

void Cpu::opCb_0xed() {} 

void Cpu::opCb_0xee() {} 

void Cpu::opCb_0xef() {} 

void Cpu::opCb_0xf0() {} 

void Cpu::opCb_0xf1() {} 

void Cpu::opCb_0xf2() {} 

void Cpu::opCb_0xf3() {} 

void Cpu::opCb_0xf4() {} 

void Cpu::opCb_0xf5() {} 

void Cpu::opCb_0xf6() {} 

void Cpu::opCb_0xf7() {} 

void Cpu::opCb_0xf8() {} 

void Cpu::opCb_0xf9() {} 

void Cpu::opCb_0xfa() {} 

void Cpu::opCb_0xfb() {} 

void Cpu::opCb_0xfc() {} 

void Cpu::opCb_0xfd() {} 

void Cpu::opCb_0xfe() {} 

void Cpu::opCb_0xff() {}
