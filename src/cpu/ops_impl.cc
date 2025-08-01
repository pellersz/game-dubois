#include "cpu.h"
#include "cpu.cc"

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
