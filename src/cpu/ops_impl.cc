#include "cpu.h"
#include "cpu.cc"

// load 
void Cpu::ld(unsigned char& dest, unsigned char src) { dest = src; } 

// arithmetic

void Cpu::opAdd(unsigned char val) 
{
    unsigned char tmp = A;
    A += val;
    setCF(A < tmp); 
    setZF(!A); 
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

void Cpu::opAdd(unsigned short val) 
{
    unsigned char tmp = getHL();
    unsigned char new_val = tmp + val;
    setHL(new_val); 
    setCF(new_val < tmp); 
    setZF(!new_val); 
    setHF((A & 0b111111111111u) < (tmp & 0b111111111111u));
    setNF(false);
}

void Cpu::opAdc(unsigned char val) 
{
    unsigned char tmp = A;
    A += val + getCF(); 
    setCF((A < tmp) || (A == tmp && getCF()));
    setZF(!A);
    // TODO: see if this actually does what you want it to
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

void Cpu::opSub(unsigned char val) 
{
    unsigned char tmp = A;
    A -= val; 
    setCF(A > tmp); 
    setZF(!A); 
    setHF((A & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}

void Cpu::opSbc(unsigned char val) 
{
    unsigned char tmp = A;
    A -= val + getCF(); 
    setCF(tmp < val + getCF()); 
    setZF(!A); 
    setHF((A & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}


void Cpu::opCp(unsigned char val) 
{
    unsigned char tmp = A - val;
    setCF(A < tmp); 
    setZF(!tmp); 
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(true);
}

void Cpu::opInc(unsigned char& dest) {
    unsigned char tmp = dest++;
    setZF(!dest); 
    setHF((dest & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

void Cpu::opInc(unsigned short& dest) { ++dest; }

void Cpu::opInc(unsigned char& lo, unsigned char& hi) { hi += !(lo += 1); }

// TODO: decide what to do with this
//void Cpu::opInc(std::function<unsigned short()> get, std::function<void(unsigned short)> set) { set(get() + 1); }


void Cpu::opDec(unsigned char& dest) {
    unsigned char tmp = dest--;
    dest -= 1; 
    setZF(!dest); 
    setHF((dest & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}

void Cpu::opDec(unsigned short& dest) { ++dest; }

void Cpu::opDec(unsigned char& lo, unsigned char& hi) { hi -= ((lo -= 1) == (unsigned char)(-1)); }
// TODO: like above
//void Cpu::opDec(std::function<unsigned short()> get, std::function<void(unsigned short)> set) { set(get() - 1); }


// logic

void Cpu::opAnd(unsigned char val) 
{
    A &= val; 
    F &= NULLF_MASK;
    setHF(true);
    setZF(!A); 
}

void Cpu::opOr(unsigned char val) 
{
    A &= val; 
    F &= NULLF_MASK;
    setZF(!A); 
}

void Cpu::opXor(unsigned char val)
{
    A ^= val; 
    F &= NULLF_MASK;
    setZF(!A); 
}

void Cpu::opCpl(unsigned char)
{
    A = ~A;
    F |= NF_MASK | HF_MASK;
}

//bit
void Cpu::opBit(unsigned char shift, unsigned char val) 
{
    setZF(val & 1u << shift);
    setNF(false);
    setHF(false);
}

void Cpu::opRes(unsigned char shift, unsigned char& dest) { dest &= ~(1u << shift); } 

void Cpu::opSet(unsigned char shift, unsigned char& dest) { dest |= (1u << shift); }

// bitshift

void Cpu::opRl(unsigned char& dest) 
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

void Cpu::opRlc(unsigned char& dest)
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

void Cpu::opRr(unsigned char& dest)
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

void Cpu::opRrc(unsigned char& dest)
{
    bool last_set = dest & 0b00000001u;
    dest = (dest >> 1) + last_set * 0b10000000u;
    setCF(last_set);
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opRrca(unsigned char&)
{
    bool new_CF = A & 0b00000001u;
    A = (A >> 1) + getCF() * 0b10000000u;
    setCF(new_CF);
    setZF(!A);
    F &= CF_MASK;
}

void Cpu::opSla(unsigned char& dest)
{
    setCF(dest & 0b10000000u);
    dest <<= 1;
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opSra(unsigned char& dest)
{
    setCF(dest & 0b00000001u);
    dest = (dest & 0b10000000u) + (dest >> 1);
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opSrl(unsigned char& dest)
{
    setCF(dest & 0b00000001u);
    dest >>= 1;
    setZF(!dest);
    F &= ZF_MASK | CF_MASK;
}

void Cpu::opSwap(unsigned char& dest)
{
    dest = (dest << 4) + (dest >> 4);
    setZF(!dest);
    F &= ZF_MASK;
}

// jumps and subroutine

void Cpu::opCall(unsigned short addr)
{
    
}   

void Cpu::opJp(unsigned short addr)
{

}

void Cpu::opJr(unsigned short addr)
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
    unsigned short tmp = SP;
    SP += val;
    F &= HF_MASK | CF_MASK;
    setCF((SP & 0b11111111) < (tmp & 0b11111111)); 
    setHF((SP & 0b1111u) < (tmp & 0b1111u));
}

// TODO: do this after memory management is done
void Cpu::opPop(unsigned short& reg)
{
    
}

// TODO: do this after memory management is done
void Cpu::opPush(unsigned short reg)
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
    unsigned char tmp = A;
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
