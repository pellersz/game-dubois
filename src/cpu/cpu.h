#ifndef CPU_H 
#define CPU_H

#include "types.h"
#include "mem.h"
#include <string>

#define C(no) (Cpu::CLOCKS_BETWEEN_EXEC * no)

class Scheduler;

class Cpu {
public:
    static const u8 CLOCKS_BETWEEN_EXEC = 4;

    Cpu(Memory&, Scheduler&);
    ~Cpu();
    void executeNext();
    // TODO: put this back
    bool handleInterupts();
    void setPC(word);
    void test(std::string);
    std::string toString();
    word getPC();
    std::string getAsm();
        
protected:
    // I would encapsulate this, but it would mean writing 10000 getters and setters
    static const byte CF_MASK    = 0b00011111;
    static const byte HF_MASK    = 0b00101111;
    static const byte NF_MASK    = 0b01001111;
    static const byte ZF_MASK    = 0b10001111;
    static const byte NULLF_MASK = 0b00001111;
    static const byte ALLF_MASK  = 0b11111111;

    byte a;
    byte f;
    byte b;
    byte c;
    byte d;
    byte e;
    byte h;
    byte l;
    word sp = 0xa000u;
    word pc = 0;
    bool ime = false;
    bool halted = false;

    Memory& memory;
    Scheduler& scheduler;
    // while the cpu does not communicate with the joypad directly, the implementation will be simpler like this

    word getAF();
    word getBC();
    word getDE();
    word getHL();
    bool getZF();
    bool getNF();
    bool getHF();
    bool getCF();
    void setAF(word);
    void setBC(word);
    void setDE(word);
    void setHL(word);
    void setZF(bool);
    void setNF(bool);
    void setHF(bool);
    void setCF(bool);

    void stackStep();
    void stackStepBack();
    void stack2Step();
    void stack2StepBack();
    void programCounterStep(u8);

    void executeRegular(byte);
    void executeBC(byte);
    
    void writtenToMemory(unsigned short);

    // load
    void opLd(byte&, byte);
    void opLd(word&, word);
    void opLd(byte&, byte&, word);

    // arithmetic
    void opAdd(byte); 
    void opAdd(word);
    void opAddSP(offs);
    void opAdc(byte);
    void opSub(byte);
    void opSbc(byte);
    void opCp(byte);
    void opInc(byte&);
    void opInc(word&);
    void opInc(byte&, byte&);
    void opDec(byte&);
    void opDec(word&);
    void opDec(byte&, byte&);

    // logic
    void opAnd(byte);
    void opOr(byte);
    void opXor(byte);
    void opCpl();

    // bit
    // the first byte of these should be a number between 0-7
    void opBit(byte, byte);
    void opRes(byte, byte&);
    void opSet(byte, byte&);

    // bitshift
    void opRl(byte&);
    void opRla();
    void opRlc(byte&);
    void opRlca();
    void opRr(byte&);
    void opRra();
    void opRrc(byte&);
    void opRrca();
    void opSla(byte&);
    void opSra(byte&);
    void opSrl(byte&);
    void opSwap(byte&);

    // jump and subroutine
    void opCall(word);
    void opCall(bool, word);
    void opJp(word);
    void opJp(bool, word);
    void opJr(offs);
    void opJr(bool, offs);
    void opRet();
    void opRet(bool);
    void opReti();
    void opRst(u8);

    // carry flag
    void opCcf();
    void opScf();

    // stack manipulation
    void opAdd(offs);  //signed offset add to SP
    void opPop(word&);
    void opPop(byte&, byte&);
    void opPush(word);
    
    // interrup-related
    void opDi();
    void opEi();
    void opHalt();

    // misc
    void opDaa();
    void opNop();
    void opStop();

    // yup, i'll be implementing this shit
    void op_0x00();  void op_0x01();  void op_0x02();  void op_0x03();  void op_0x04();  void op_0x05();  void op_0x06();  void op_0x07(); 
    void op_0x08();  void op_0x09();  void op_0x0a();  void op_0x0b();  void op_0x0c();  void op_0x0d();  void op_0x0e();  void op_0x0f(); 
    void op_0x10();  void op_0x11();  void op_0x12();  void op_0x13();  void op_0x14();  void op_0x15();  void op_0x16();  void op_0x17(); 
    void op_0x18();  void op_0x19();  void op_0x1a();  void op_0x1b();  void op_0x1c();  void op_0x1d();  void op_0x1e();  void op_0x1f(); 
    void op_0x20();  void op_0x21();  void op_0x22();  void op_0x23();  void op_0x24();  void op_0x25();  void op_0x26();  void op_0x27(); 
    void op_0x28();  void op_0x29();  void op_0x2a();  void op_0x2b();  void op_0x2c();  void op_0x2d();  void op_0x2e();  void op_0x2f(); 
    void op_0x30();  void op_0x31();  void op_0x32();  void op_0x33();  void op_0x34();  void op_0x35();  void op_0x36();  void op_0x37(); 
    void op_0x38();  void op_0x39();  void op_0x3a();  void op_0x3b();  void op_0x3c();  void op_0x3d();  void op_0x3e();  void op_0x3f(); 
    void op_0x40();  void op_0x41();  void op_0x42();  void op_0x43();  void op_0x44();  void op_0x45();  void op_0x46();  void op_0x47(); 
    void op_0x48();  void op_0x49();  void op_0x4a();  void op_0x4b();  void op_0x4c();  void op_0x4d();  void op_0x4e();  void op_0x4f(); 
    void op_0x50();  void op_0x51();  void op_0x52();  void op_0x53();  void op_0x54();  void op_0x55();  void op_0x56();  void op_0x57(); 
    void op_0x58();  void op_0x59();  void op_0x5a();  void op_0x5b();  void op_0x5c();  void op_0x5d();  void op_0x5e();  void op_0x5f(); 
    void op_0x60();  void op_0x61();  void op_0x62();  void op_0x63();  void op_0x64();  void op_0x65();  void op_0x66();  void op_0x67(); 
    void op_0x68();  void op_0x69();  void op_0x6a();  void op_0x6b();  void op_0x6c();  void op_0x6d();  void op_0x6e();  void op_0x6f(); 
    void op_0x70();  void op_0x71();  void op_0x72();  void op_0x73();  void op_0x74();  void op_0x75();  void op_0x76();  void op_0x77(); 
    void op_0x78();  void op_0x79();  void op_0x7a();  void op_0x7b();  void op_0x7c();  void op_0x7d();  void op_0x7e();  void op_0x7f(); 
    void op_0x80();  void op_0x81();  void op_0x82();  void op_0x83();  void op_0x84();  void op_0x85();  void op_0x86();  void op_0x87(); 
    void op_0x88();  void op_0x89();  void op_0x8a();  void op_0x8b();  void op_0x8c();  void op_0x8d();  void op_0x8e();  void op_0x8f(); 
    void op_0x90();  void op_0x91();  void op_0x92();  void op_0x93();  void op_0x94();  void op_0x95();  void op_0x96();  void op_0x97(); 
    void op_0x98();  void op_0x99();  void op_0x9a();  void op_0x9b();  void op_0x9c();  void op_0x9d();  void op_0x9e();  void op_0x9f(); 
    void op_0xa0();  void op_0xa1();  void op_0xa2();  void op_0xa3();  void op_0xa4();  void op_0xa5();  void op_0xa6();  void op_0xa7(); 
    void op_0xa8();  void op_0xa9();  void op_0xaa();  void op_0xab();  void op_0xac();  void op_0xad();  void op_0xae();  void op_0xaf(); 
    void op_0xb0();  void op_0xb1();  void op_0xb2();  void op_0xb3();  void op_0xb4();  void op_0xb5();  void op_0xb6();  void op_0xb7(); 
    void op_0xb8();  void op_0xb9();  void op_0xba();  void op_0xbb();  void op_0xbc();  void op_0xbd();  void op_0xbe();  void op_0xbf(); 
    void op_0xc0();  void op_0xc1();  void op_0xc2();  void op_0xc3();  void op_0xc4();  void op_0xc5();  void op_0xc6();  void op_0xc7(); 
    void op_0xc8();  void op_0xc9();  void op_0xca();  void op_0xcb();  void op_0xcc();  void op_0xcd();  void op_0xce();  void op_0xcf(); 
    void op_0xd0();  void op_0xd1();  void op_0xd2();  void op_0xd3();  void op_0xd4();  void op_0xd5();  void op_0xd6();  void op_0xd7(); 
    void op_0xd8();  void op_0xd9();  void op_0xda();  void op_0xdb();  void op_0xdc();  void op_0xdd();  void op_0xde();  void op_0xdf(); 
    void op_0xe0();  void op_0xe1();  void op_0xe2();  void op_0xe3();  void op_0xe4();  void op_0xe5();  void op_0xe6();  void op_0xe7(); 
    void op_0xe8();  void op_0xe9();  void op_0xea();  void op_0xeb();  void op_0xec();  void op_0xed();  void op_0xee();  void op_0xef(); 
    void op_0xf0();  void op_0xf1();  void op_0xf2();  void op_0xf3();  void op_0xf4();  void op_0xf5();  void op_0xf6();  void op_0xf7(); 
    void op_0xf8();  void op_0xf9();  void op_0xfa();  void op_0xfb();  void op_0xfc();  void op_0xfd();  void op_0xfe();  void op_0xff(); 

    void opCb_0x00();  void opCb_0x01();  void opCb_0x02();  void opCb_0x03();  void opCb_0x04();  void opCb_0x05();  void opCb_0x06();  void opCb_0x07(); 
    void opCb_0x08();  void opCb_0x09();  void opCb_0x0a();  void opCb_0x0b();  void opCb_0x0c();  void opCb_0x0d();  void opCb_0x0e();  void opCb_0x0f(); 
    void opCb_0x10();  void opCb_0x11();  void opCb_0x12();  void opCb_0x13();  void opCb_0x14();  void opCb_0x15();  void opCb_0x16();  void opCb_0x17(); 
    void opCb_0x18();  void opCb_0x19();  void opCb_0x1a();  void opCb_0x1b();  void opCb_0x1c();  void opCb_0x1d();  void opCb_0x1e();  void opCb_0x1f(); 
    void opCb_0x20();  void opCb_0x21();  void opCb_0x22();  void opCb_0x23();  void opCb_0x24();  void opCb_0x25();  void opCb_0x26();  void opCb_0x27(); 
    void opCb_0x28();  void opCb_0x29();  void opCb_0x2a();  void opCb_0x2b();  void opCb_0x2c();  void opCb_0x2d();  void opCb_0x2e();  void opCb_0x2f(); 
    void opCb_0x30();  void opCb_0x31();  void opCb_0x32();  void opCb_0x33();  void opCb_0x34();  void opCb_0x35();  void opCb_0x36();  void opCb_0x37(); 
    void opCb_0x38();  void opCb_0x39();  void opCb_0x3a();  void opCb_0x3b();  void opCb_0x3c();  void opCb_0x3d();  void opCb_0x3e();  void opCb_0x3f(); 
    void opCb_0x40();  void opCb_0x41();  void opCb_0x42();  void opCb_0x43();  void opCb_0x44();  void opCb_0x45();  void opCb_0x46();  void opCb_0x47(); 
    void opCb_0x48();  void opCb_0x49();  void opCb_0x4a();  void opCb_0x4b();  void opCb_0x4c();  void opCb_0x4d();  void opCb_0x4e();  void opCb_0x4f(); 
    void opCb_0x50();  void opCb_0x51();  void opCb_0x52();  void opCb_0x53();  void opCb_0x54();  void opCb_0x55();  void opCb_0x56();  void opCb_0x57(); 
    void opCb_0x58();  void opCb_0x59();  void opCb_0x5a();  void opCb_0x5b();  void opCb_0x5c();  void opCb_0x5d();  void opCb_0x5e();  void opCb_0x5f(); 
    void opCb_0x60();  void opCb_0x61();  void opCb_0x62();  void opCb_0x63();  void opCb_0x64();  void opCb_0x65();  void opCb_0x66();  void opCb_0x67(); 
    void opCb_0x68();  void opCb_0x69();  void opCb_0x6a();  void opCb_0x6b();  void opCb_0x6c();  void opCb_0x6d();  void opCb_0x6e();  void opCb_0x6f(); 
    void opCb_0x70();  void opCb_0x71();  void opCb_0x72();  void opCb_0x73();  void opCb_0x74();  void opCb_0x75();  void opCb_0x76();  void opCb_0x77(); 
    void opCb_0x78();  void opCb_0x79();  void opCb_0x7a();  void opCb_0x7b();  void opCb_0x7c();  void opCb_0x7d();  void opCb_0x7e();  void opCb_0x7f(); 
    void opCb_0x80();  void opCb_0x81();  void opCb_0x82();  void opCb_0x83();  void opCb_0x84();  void opCb_0x85();  void opCb_0x86();  void opCb_0x87(); 
    void opCb_0x88();  void opCb_0x89();  void opCb_0x8a();  void opCb_0x8b();  void opCb_0x8c();  void opCb_0x8d();  void opCb_0x8e();  void opCb_0x8f(); 
    void opCb_0x90();  void opCb_0x91();  void opCb_0x92();  void opCb_0x93();  void opCb_0x94();  void opCb_0x95();  void opCb_0x96();  void opCb_0x97(); 
    void opCb_0x98();  void opCb_0x99();  void opCb_0x9a();  void opCb_0x9b();  void opCb_0x9c();  void opCb_0x9d();  void opCb_0x9e();  void opCb_0x9f(); 
    void opCb_0xa0();  void opCb_0xa1();  void opCb_0xa2();  void opCb_0xa3();  void opCb_0xa4();  void opCb_0xa5();  void opCb_0xa6();  void opCb_0xa7(); 
    void opCb_0xa8();  void opCb_0xa9();  void opCb_0xaa();  void opCb_0xab();  void opCb_0xac();  void opCb_0xad();  void opCb_0xae();  void opCb_0xaf(); 
    void opCb_0xb0();  void opCb_0xb1();  void opCb_0xb2();  void opCb_0xb3();  void opCb_0xb4();  void opCb_0xb5();  void opCb_0xb6();  void opCb_0xb7(); 
    void opCb_0xb8();  void opCb_0xb9();  void opCb_0xba();  void opCb_0xbb();  void opCb_0xbc();  void opCb_0xbd();  void opCb_0xbe();  void opCb_0xbf(); 
    void opCb_0xc0();  void opCb_0xc1();  void opCb_0xc2();  void opCb_0xc3();  void opCb_0xc4();  void opCb_0xc5();  void opCb_0xc6();  void opCb_0xc7(); 
    void opCb_0xc8();  void opCb_0xc9();  void opCb_0xca();  void opCb_0xcb();  void opCb_0xcc();  void opCb_0xcd();  void opCb_0xce();  void opCb_0xcf(); 
    void opCb_0xd0();  void opCb_0xd1();  void opCb_0xd2();  void opCb_0xd3();  void opCb_0xd4();  void opCb_0xd5();  void opCb_0xd6();  void opCb_0xd7(); 
    void opCb_0xd8();  void opCb_0xd9();  void opCb_0xda();  void opCb_0xdb();  void opCb_0xdc();  void opCb_0xdd();  void opCb_0xde();  void opCb_0xdf(); 
    void opCb_0xe0();  void opCb_0xe1();  void opCb_0xe2();  void opCb_0xe3();  void opCb_0xe4();  void opCb_0xe5();  void opCb_0xe6();  void opCb_0xe7(); 
    void opCb_0xe8();  void opCb_0xe9();  void opCb_0xea();  void opCb_0xeb();  void opCb_0xec();  void opCb_0xed();  void opCb_0xee();  void opCb_0xef(); 
    void opCb_0xf0();  void opCb_0xf1();  void opCb_0xf2();  void opCb_0xf3();  void opCb_0xf4();  void opCb_0xf5();  void opCb_0xf6();  void opCb_0xf7(); 
    void opCb_0xf8();  void opCb_0xf9();  void opCb_0xfa();  void opCb_0xfb();  void opCb_0xfc();  void opCb_0xfd();  void opCb_0xfe();  void opCb_0xff(); 
};

#endif
