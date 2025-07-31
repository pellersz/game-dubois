#include "cpu.h"
#include "cpu.cc"

// load 
inline void Cpu::ld(unsigned char& dest, unsigned char src) { dest = src; } 

// arithmetic

inline void Cpu::op_add(unsigned char val) 
{
    unsigned char tmp = A;
    A += val; 
    if (A < tmp) 
       setCF(true); 
    if (!A)
       setZF(true); 
    if ((A & 0b1111u) < (tmp & 0b1111u)) 
        setHF(true);
    setNF(false);
}

inline void Cpu::op_adc(unsigned char val) 
{
    unsigned char tmp = A;
    A += val + getCF(); 
    if (A < tmp) 
        setCF(true);
    if (!A)
        setZF(true);
    // TODO: see if this actually does what you want it to
    if ((A & 0b1111u) < (tmp & 0b1111u)) 
        setHF(true);
    setNF(false);
}

inline void Cpu::op_sub(unsigned char val) 
{
    unsigned char tmp = A;
    A -= val; 
    if (A > tmp) 
       setCF(true); 
    if (!A)
       setZF(true); 
    if ((A & 0b1111u) > (tmp & 0b1111u)) 
        setHF(true);
    setNF(true);
}

inline void Cpu::op_sbc(unsigned char val) 
{
    unsigned char tmp = A;
    A -= val + getCF(); 
    if (A > tmp) 
       setCF(true); 
    if (!A)
       setZF(true); 
    if ((A & 0b1111u) > (tmp & 0b1111u)) 
        setHF(true);
    setNF(true);
}


inline void Cpu::op_cp(unsigned char val) 
{
    unsigned char tmp = A - val;
    if (A < tmp) 
       setCF(true); 
    if (!tmp)
       setZF(true); 
    if ((A & 0b1111u) < (tmp & 0b1111u)) 
        setHF(true);
    setNF(true);
}

inline void Cpu::op_inc(unsigned char& dest) {
    unsigned char tmp = dest++;
    if (!dest)
       setZF(true); 
    if ((dest & 0b1111u) < (tmp & 0b1111u)) 
        setHF(true);
    setNF(false);
}

inline void Cpu::op_dec(unsigned char& dest) {
    unsigned char tmp = dest--;
    dest -= 1; 
    if (!dest)
       setZF(true); 
    if ((dest & 0b1111u) > (tmp & 0b1111u)) 
        setHF(true);
    setNF(true);
}

inline void Cpu::op_add(unsigned short val) 
{
    unsigned char tmp = getHL();
    unsigned char new_val = tmp + val;
    setHL(new_val); 
    if (new_val < tmp) 
       setCF(true); 
    if (!new_val)
       setZF(true); 
    if ((A & 0b111111111111u) < (tmp & 0b111111111111u)) 
        setHF(true);
    setNF(false);
}

inline void Cpu::op_inc(std::function<unsigned short()> get, std::function<void(unsigned short)> set) { set(get() + 1); }

inline void Cpu::op_dec(std::function<unsigned short()> get, std::function<void(unsigned short)> set) { set(get() - 1); }


// logic

inline void Cpu::op_and(unsigned char val) 
{
    A &= val; 
    F = 0b0100u;
    if(!A)
       setZF(true); 
}

inline void Cpu::op_or(unsigned char val) 
{
    A &= val; 
    F = 0b0000u;
    if(!A)
       setZF(true); 
}

inline void Cpu::op_xor(unsigned char val)
{
    A ^= val; 
    F = 0b0000u;
    if(!A)
       setZF(true); 

}

inline void Cpu::op_cpl(unsigned char)
{
    A = ~A;
    F |= 0b0110u;
}

//bit
inline void Cpu::op_bit(unsigned char val) 
{
    if (val & 0b00001000u) 
        setZF(true);
    F |= 0b0010u;
}

inline void Cpu::op_res(unsigned char& dest) { dest &= 0b11110111u; } 

inline void Cpu::op_set(unsigned char& dest) { dest |= 0b00001000u; }

// bitshift

inline void Cpu::op_rl(unsigned char& dest) 
{
    bool new_CF = dest & 0b10000000u;
    dest <<= 1;
    dest += getCF();
    setCF(new_CF);
    if (!dest) 
        setZF(true);
    F ^= 0b1001u;
}

inline void Cpu::op_rla()
{
    bool new_CF = A & 0b10000000u;
    A <<= 1;
    A += getCF();
    setCF(new_CF);
    if (!A) 
        setZF(true);
    F ^= 0b1000u;
}

inline void Cpu::op_rlc(unsigned char& dest)
{
    bool last_set = dest & 0b10000000u;
    dest <<= 1;
    dest += last_set;
    setCF(last_set);
    if (!dest) 
        setZF(true);
    F ^= 0b1001u;
}

inline void Cpu::op_rlca()
{
    bool last_set = A & 0b10000000u;
    A <<= 1;
    A += last_set;
    setCF(last_set);
    if (!A) 
        setZF(true);
    F ^= 0b1000u;
}

inline void Cpu::op_rr(unsigned char& dest)
{
    bool new_CF = dest & 0b00000001u;
    dest >>= 1;
    dest += getCF() * 0b10000000u;
    setCF(new_CF);
    if (!dest) 
        setZF(true);
    F ^= 0b1001u;
}

inline void Cpu::op_rra()
{
    bool new_CF = A & 0b00000001u;
    A >>= 1;
    A += getCF() * 0b10000000u;
    setCF(new_CF);
    if (!A) 
        setZF(true);
    F ^= 0b1000u;
}

inline void Cpu::op_rrc(unsigned char& dest)
{
    bool last_set = dest & 0b00000001u;
    dest >>= 1;
    dest += last_set * 0b10000000u;
    setCF(last_set);
    if (!dest) 
        setZF(true);
    F ^= 0b1001u;
}

inline void Cpu::op_rrca(unsigned char&)
{
    bool new_CF = A & 0b00000001u;
    A >>= 1;
    A += getCF() * 0b10000000u;
    setCF(new_CF);
    if (!A) 
        setZF(true);
    F ^= 0b1000u;
}

inline void Cpu::op_sla(unsigned char& dest)
{
    setCF(dest & 0b10000000u);
    dest <<= 1;
    if (!dest) 
        setZF(true);
    F ^= 0b1001u;
}

inline void Cpu::op_sra(unsigned char& dest)
{
    setCF(dest & 0b00000001u);
    dest >>= 1;
    if (!dest) 
        setZF(true);
    F ^= 0b1001u;
}

inline void Cpu::op_srl(unsigned char&)
{

}

inline void Cpu::swap(unsigned char&)
{

}

