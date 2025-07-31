#include "cpu.h"
#include "cpu.cc"

// load 
inline void Cpu::ld(unsigned char& dest, unsigned char src) { dest = src; } 

// arithmetic

inline void Cpu::op_add(unsigned char val) 
{
    unsigned char tmp = A;
    setCF(A < tmp); 
    setZF(!A); 
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

inline void Cpu::op_adc(unsigned char val) 
{
    unsigned char tmp = A;
    A += val + getCF(); 
    setCF((A < tmp) || (A == tmp && getCF()));
    setZF(!A);
    // TODO: see if this actually does what you want it to
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

inline void Cpu::op_sub(unsigned char val) 
{
    unsigned char tmp = A;
    A -= val; 
    setCF(A > tmp); 
    setZF(!A); 
    setHF((A & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}

inline void Cpu::op_sbc(unsigned char val) 
{
    unsigned char tmp = A;
    A -= val + getCF(); 
    setCF(tmp < val + getCF()); 
    setZF(!A); 
    setHF((A & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}


inline void Cpu::op_cp(unsigned char val) 
{
    unsigned char tmp = A - val;
    setCF(A < tmp); 
    setZF(!tmp); 
    setHF((A & 0b1111u) < (tmp & 0b1111u));
    setNF(true);
}

inline void Cpu::op_inc(unsigned char& dest) {
    unsigned char tmp = dest++;
    setZF(!dest); 
    setHF((dest & 0b1111u) < (tmp & 0b1111u));
    setNF(false);
}

inline void Cpu::op_dec(unsigned char& dest) {
    unsigned char tmp = dest--;
    dest -= 1; 
    setZF(!dest); 
    setHF((dest & 0b1111u) > (tmp & 0b1111u));
    setNF(true);
}

inline void Cpu::op_add(unsigned short val) 
{
    unsigned char tmp = getHL();
    unsigned char new_val = tmp + val;
    setHL(new_val); 
    setCF(new_val < tmp); 
    setZF(!new_val); 
    setHF((A & 0b111111111111u) < (tmp & 0b111111111111u));
    setNF(false);
}

inline void Cpu::op_inc(std::function<unsigned short()> get, std::function<void(unsigned short)> set) { set(get() + 1); }

inline void Cpu::op_dec(std::function<unsigned short()> get, std::function<void(unsigned short)> set) { set(get() - 1); }


// logic

inline void Cpu::op_and(unsigned char val) 
{
    A &= val; 
    F = 0b0100u;
    setZF(!A); 
}

inline void Cpu::op_or(unsigned char val) 
{
    A &= val; 
    F = 0b0000u;
    setZF(!A); 
}

inline void Cpu::op_xor(unsigned char val)
{
    A ^= val; 
    F = 0b0000u;
    setZF(!A); 
}

inline void Cpu::op_cpl(unsigned char)
{
    A = ~A;
    F |= 0b0110u;
}

//bit
inline void Cpu::op_bit(unsigned char val) 
{
    setZF(val & 0b00001000u);
    setNF(false);
    setHF(false);
}

inline void Cpu::op_res(unsigned char& dest) { dest &= 0b11110111u; } 

inline void Cpu::op_set(unsigned char& dest) { dest |= 0b00001000u; }

// bitshift

inline void Cpu::op_rl(unsigned char& dest) 
{
    bool new_CF = dest & 0b10000000u;
    dest = (dest << 1) + getCF();
    setCF(new_CF);
    setZF(!dest);
    F ^= 0b1001u;
}

inline void Cpu::op_rla()
{
    bool new_CF = A & 0b10000000u;
    A = (A << 1) + getCF();
    setCF(new_CF);
    setZF(!A);
    F ^= 0b1000u;
}

inline void Cpu::op_rlc(unsigned char& dest)
{
    bool last_set = dest & 0b10000000u;
    dest = (dest << 1) + last_set;
    setCF(last_set);
    setZF(!dest);
    F ^= 0b1001u;
}

inline void Cpu::op_rlca()
{
    bool last_set = A & 0b10000000u;
    A = (A << 1) + last_set;
    setCF(last_set);
    setZF(!A);
    F ^= 0b1000u;
}

inline void Cpu::op_rr(unsigned char& dest)
{
    bool new_CF = dest & 0b00000001u;
    dest = (dest >> 1) + getCF() * 0b10000000u;
    setCF(new_CF);
    setZF(!dest);
    F ^= 0b1001u;
}

inline void Cpu::op_rra()
{
    bool new_CF = A & 0b00000001u;
    A = (A >> 1) + getCF() * 0b10000000u;
    setCF(new_CF);
    setZF(!A);
    F ^= 0b1000u;
}

inline void Cpu::op_rrc(unsigned char& dest)
{
    bool last_set = dest & 0b00000001u;
    dest = (dest >> 1) + last_set * 0b10000000u;
    setCF(last_set);
    setZF(!dest);
    F ^= 0b1001u;
}

inline void Cpu::op_rrca(unsigned char&)
{
    bool new_CF = A & 0b00000001u;
    A = (A >> 1) + getCF() * 0b10000000u;
    setCF(new_CF);
    setZF(!A);
    F ^= 0b1000u;
}

inline void Cpu::op_sla(unsigned char& dest)
{
    setCF(dest & 0b10000000u);
    dest <<= 1;
    setZF(!dest);
    F ^= 0b1001u;
}

inline void Cpu::op_sra(unsigned char& dest)
{
    setCF(dest & 0b00000001u);
    dest = (dest & 0b10000000u) + (dest >> 1);
    setZF(!dest);
    F ^= 0b1001u;
}

inline void Cpu::op_srl(unsigned char& dest)
{
    setCF(dest & 0b00000001u);
    dest >>= 1;
    setZF(!dest);
    F ^= 0b1001u;
}

inline void Cpu::swap(unsigned char& dest)
{
    dest = (dest << 4) + (dest >> 4);
    setZF(!dest);
    F ^= 0b0001u;
}

