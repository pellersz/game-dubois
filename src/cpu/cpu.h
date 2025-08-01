#ifndef CPU 
#define CPU

typedef unsigned char byte;
typedef unsigned short word;
typedef char offs;

class Cpu {
public:
    Cpu();
    ~Cpu();

protected:
    // I would encapsulate this, but it would mean writing 10000 getters and setters
    static const byte ZF_MASK = 0b00011111u;
    static const byte NF_MASK = 0b00101111u;
    static const byte HF_MASK = 0b01001111u;
    static const byte CF_MASK = 0b10001111u;
    static const byte NULLF_MASK = 0b00001111u;
    static const byte ALLF_MASK = 0b11111111u;

    byte A;
    byte F;
    byte B;
    byte C;
    byte D;
    byte E;
    byte H;
    byte L;
    word SP = 0xa000u;
  //word SP = 0xdfffu;
    word PC = 0;
    bool IME = false;

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

    byte getIF();
    byte getIE();

    void stackStep();
    void stackStepBack();
    void programCounterStep();

    void executeRegular(byte);
    void executeBC(byte);

    // load
    void ld(byte&, byte);
    void ld(word&, word);

    // arithmetic
    void opAdd(byte); 
    void opAdd(word);
    void opAdc(byte);
    void opSub(byte);
    void opSbc(byte);
    void opCp(byte);
    void opInc(byte&);
    void opInc(word&);
    void opInc(byte&, byte&);
    // TODO: decide what to do with these)
    // void opInc(std::function<word()>, std::function<void(word)>);
    void opDec(byte&);
    void opDec(word&);
    void opDec(byte&, byte&);
    // TODO: like above
    // void opDec(std::function<word()>, std::function<void(word)>);

    // logic
    void opAnd(byte);
    void opOr(byte);
    void opXor(byte);
    void opCpl(byte);

    // bit
    // the first char of this should be a number between 0-7
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
    void opRrca(byte&);
    void opSla(byte&);
    void opSra(byte&);
    void opSrl(byte&);
    void opSwap(byte&);

    // jump and subroutine
    void opCall(word);
    void opJp(word);
    void opJr(word);
    void opRet();
    void opReti();
    void opRst();

    // carry flag
    void opCcf();
    void opScf();

    // stack manipulation
    void opAdd(char);  //signed offset add to SP
    void opPop(word&);
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

    void op_cb_0x00();  void op_cb_0x01();  void op_cb_0x02();  void op_cb_0x03();  void op_cb_0x04();  void op_cb_0x05();  void op_cb_0x06();  void op_cb_0x07(); 
    void op_cb_0x08();  void op_cb_0x09();  void op_cb_0x0a();  void op_cb_0x0b();  void op_cb_0x0c();  void op_cb_0x0d();  void op_cb_0x0e();  void op_cb_0x0f(); 
    void op_cb_0x10();  void op_cb_0x11();  void op_cb_0x12();  void op_cb_0x13();  void op_cb_0x14();  void op_cb_0x15();  void op_cb_0x16();  void op_cb_0x17(); 
    void op_cb_0x18();  void op_cb_0x19();  void op_cb_0x1a();  void op_cb_0x1b();  void op_cb_0x1c();  void op_cb_0x1d();  void op_cb_0x1e();  void op_cb_0x1f(); 
    void op_cb_0x20();  void op_cb_0x21();  void op_cb_0x22();  void op_cb_0x23();  void op_cb_0x24();  void op_cb_0x25();  void op_cb_0x26();  void op_cb_0x27(); 
    void op_cb_0x28();  void op_cb_0x29();  void op_cb_0x2a();  void op_cb_0x2b();  void op_cb_0x2c();  void op_cb_0x2d();  void op_cb_0x2e();  void op_cb_0x2f(); 
    void op_cb_0x30();  void op_cb_0x31();  void op_cb_0x32();  void op_cb_0x33();  void op_cb_0x34();  void op_cb_0x35();  void op_cb_0x36();  void op_cb_0x37(); 
    void op_cb_0x38();  void op_cb_0x39();  void op_cb_0x3a();  void op_cb_0x3b();  void op_cb_0x3c();  void op_cb_0x3d();  void op_cb_0x3e();  void op_cb_0x3f(); 
    void op_cb_0x40();  void op_cb_0x41();  void op_cb_0x42();  void op_cb_0x43();  void op_cb_0x44();  void op_cb_0x45();  void op_cb_0x46();  void op_cb_0x47(); 
    void op_cb_0x48();  void op_cb_0x49();  void op_cb_0x4a();  void op_cb_0x4b();  void op_cb_0x4c();  void op_cb_0x4d();  void op_cb_0x4e();  void op_cb_0x4f(); 
    void op_cb_0x50();  void op_cb_0x51();  void op_cb_0x52();  void op_cb_0x53();  void op_cb_0x54();  void op_cb_0x55();  void op_cb_0x56();  void op_cb_0x57(); 
    void op_cb_0x58();  void op_cb_0x59();  void op_cb_0x5a();  void op_cb_0x5b();  void op_cb_0x5c();  void op_cb_0x5d();  void op_cb_0x5e();  void op_cb_0x5f(); 
    void op_cb_0x60();  void op_cb_0x61();  void op_cb_0x62();  void op_cb_0x63();  void op_cb_0x64();  void op_cb_0x65();  void op_cb_0x66();  void op_cb_0x67(); 
    void op_cb_0x68();  void op_cb_0x69();  void op_cb_0x6a();  void op_cb_0x6b();  void op_cb_0x6c();  void op_cb_0x6d();  void op_cb_0x6e();  void op_cb_0x6f(); 
    void op_cb_0x70();  void op_cb_0x71();  void op_cb_0x72();  void op_cb_0x73();  void op_cb_0x74();  void op_cb_0x75();  void op_cb_0x76();  void op_cb_0x77(); 
    void op_cb_0x78();  void op_cb_0x79();  void op_cb_0x7a();  void op_cb_0x7b();  void op_cb_0x7c();  void op_cb_0x7d();  void op_cb_0x7e();  void op_cb_0x7f(); 
    void op_cb_0x80();  void op_cb_0x81();  void op_cb_0x82();  void op_cb_0x83();  void op_cb_0x84();  void op_cb_0x85();  void op_cb_0x86();  void op_cb_0x87(); 
    void op_cb_0x88();  void op_cb_0x89();  void op_cb_0x8a();  void op_cb_0x8b();  void op_cb_0x8c();  void op_cb_0x8d();  void op_cb_0x8e();  void op_cb_0x8f(); 
    void op_cb_0x90();  void op_cb_0x91();  void op_cb_0x92();  void op_cb_0x93();  void op_cb_0x94();  void op_cb_0x95();  void op_cb_0x96();  void op_cb_0x97(); 
    void op_cb_0x98();  void op_cb_0x99();  void op_cb_0x9a();  void op_cb_0x9b();  void op_cb_0x9c();  void op_cb_0x9d();  void op_cb_0x9e();  void op_cb_0x9f(); 
    void op_cb_0xa0();  void op_cb_0xa1();  void op_cb_0xa2();  void op_cb_0xa3();  void op_cb_0xa4();  void op_cb_0xa5();  void op_cb_0xa6();  void op_cb_0xa7(); 
    void op_cb_0xa8();  void op_cb_0xa9();  void op_cb_0xaa();  void op_cb_0xab();  void op_cb_0xac();  void op_cb_0xad();  void op_cb_0xae();  void op_cb_0xaf(); 
    void op_cb_0xb0();  void op_cb_0xb1();  void op_cb_0xb2();  void op_cb_0xb3();  void op_cb_0xb4();  void op_cb_0xb5();  void op_cb_0xb6();  void op_cb_0xb7(); 
    void op_cb_0xb8();  void op_cb_0xb9();  void op_cb_0xba();  void op_cb_0xbb();  void op_cb_0xbc();  void op_cb_0xbd();  void op_cb_0xbe();  void op_cb_0xbf(); 
    void op_cb_0xc0();  void op_cb_0xc1();  void op_cb_0xc2();  void op_cb_0xc3();  void op_cb_0xc4();  void op_cb_0xc5();  void op_cb_0xc6();  void op_cb_0xc7(); 
    void op_cb_0xc8();  void op_cb_0xc9();  void op_cb_0xca();  void op_cb_0xcb();  void op_cb_0xcc();  void op_cb_0xcd();  void op_cb_0xce();  void op_cb_0xcf(); 
    void op_cb_0xd0();  void op_cb_0xd1();  void op_cb_0xd2();  void op_cb_0xd3();  void op_cb_0xd4();  void op_cb_0xd5();  void op_cb_0xd6();  void op_cb_0xd7(); 
    void op_cb_0xd8();  void op_cb_0xd9();  void op_cb_0xda();  void op_cb_0xdb();  void op_cb_0xdc();  void op_cb_0xdd();  void op_cb_0xde();  void op_cb_0xdf(); 
    void op_cb_0xe0();  void op_cb_0xe1();  void op_cb_0xe2();  void op_cb_0xe3();  void op_cb_0xe4();  void op_cb_0xe5();  void op_cb_0xe6();  void op_cb_0xe7(); 
    void op_cb_0xe8();  void op_cb_0xe9();  void op_cb_0xea();  void op_cb_0xeb();  void op_cb_0xec();  void op_cb_0xed();  void op_cb_0xee();  void op_cb_0xef(); 
    void op_cb_0xf0();  void op_cb_0xf1();  void op_cb_0xf2();  void op_cb_0xf3();  void op_cb_0xf4();  void op_cb_0xf5();  void op_cb_0xf6();  void op_cb_0xf7(); 
    void op_cb_0xf8();  void op_cb_0xf9();  void op_cb_0xfa();  void op_cb_0xfb();  void op_cb_0xfc();  void op_cb_0xfd();  void op_cb_0xfe();  void op_cb_0xff(); 
};

#endif
