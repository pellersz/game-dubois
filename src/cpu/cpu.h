#ifndef CPU 
#define CPU

#include <functional>
#include <type_traits>
class Cpu {
public:
    Cpu();
    ~Cpu();

protected:
    // I would encapsulate this, but it would mean writing 10000 getters and setters
    static const unsigned char ZF_MASK = 0b00011111u;
    static const unsigned char NF_MASK = 0b00101111u;
    static const unsigned char HF_MASK = 0b01001111u;
    static const unsigned char CF_MASK = 0b10001111u;
    static const unsigned char NULLF_MASK = 0b00001111u;
    static const unsigned char ALLF_MASK = 0b11111111u;

    unsigned char A;
    unsigned char F;
    unsigned char B;
    unsigned char C;
    unsigned char D;
    unsigned char E;
    unsigned char H;
    unsigned char L;
    unsigned short SP = 0xa000u;
  //unsigned short SP = 0xdfffu;
    unsigned short PC = 0;
    bool IME = false;

    inline unsigned short getAF();
    inline unsigned short getBC();
    inline unsigned short getDE();
    inline unsigned short getHL();
    inline bool getZF();
    inline bool getNF();
    inline bool getHF();
    inline bool getCF();
    inline void setAF(unsigned short);
    inline void setBC(unsigned short);
    inline void setDE(unsigned short);
    inline void setHL(unsigned short); 
    inline void setZF(bool);
    inline void setNF(bool);
    inline void setHF(bool);
    inline void setCF(bool);

    inline unsigned char getIF();
    inline unsigned char getIE();

    inline void stackStep();
    inline void stackStepBack();
    inline void programCounterStep();

    inline void executeRegular(unsigned char);
    inline void executeBC(unsigned char);

    // load
    inline void ld(unsigned char&, unsigned char);
    inline void ld(unsigned short&, unsigned short);

    // arithmetic
    inline void opAdd(unsigned char); 
    inline void opAdd(unsigned short);
    inline void opAdc(unsigned char);
    inline void opSub(unsigned char);
    inline void opSbc(unsigned char);
    inline void opCp(unsigned char);
    inline void opInc(unsigned char&);
    inline void opInc(unsigned short&);
    inline void opInc(unsigned char&, unsigned char&);
    // TODO: decide what to do with these)
    //inline void opInc(std::function<unsigned short()>, std::function<void(unsigned short)>);
    inline void opDec(unsigned char&);
    inline void opDec(unsigned short&);
    inline void opDec(unsigned char&, unsigned char&);
    // TODO: like above
    //inline void opDec(std::function<unsigned short()>, std::function<void(unsigned short)>);

    // logic
    inline void opAnd(unsigned char);
    inline void opOr(unsigned char);
    inline void opXor(unsigned char);
    inline void opCpl(unsigned char);

    // bit
    // the first char of this should be a number between 0-7
    inline void opBit(unsigned char, unsigned char);
    inline void opRes(unsigned char, unsigned char&);
    inline void opSet(unsigned char, unsigned char&);

    // bitshift
    inline void opRl(unsigned char&);
    inline void opRla();
    inline void opRlc(unsigned char&);
    inline void opRlca();
    inline void opRr(unsigned char&);
    inline void opRra();
    inline void opRrc(unsigned char&);
    inline void opRrca(unsigned char&);
    inline void opSla(unsigned char&);
    inline void opSra(unsigned char&);
    inline void opSrl(unsigned char&);
    inline void opSwap(unsigned char&);

    // jump and subroutine
    inline void opCall(unsigned short);
    inline void opJp(unsigned short);
    inline void opJr(unsigned short);
    inline void opRet();
    inline void opReti();
    inline void opRst();

    // carry flag
    inline void opCcf();
    inline void opScf();

    // stack manipulation
    inline void opAdd(char);  //signed offset add to SP
    inline void opPop(unsigned short&);
    inline void opPush(unsigned short);
    
    // interrup-related
    inline void opDi();
    inline void opEi();
    inline void opHalt();

    // misc
    inline void opDaa();
    inline void opNop();
    inline void opStop();

    // yup, i'll be implementing this shit
    inline void op_0x00(); inline void op_0x01(); inline void op_0x02(); inline void op_0x03(); inline void op_0x04(); inline void op_0x05(); inline void op_0x06(); inline void op_0x07(); 
    inline void op_0x08(); inline void op_0x09(); inline void op_0x0a(); inline void op_0x0b(); inline void op_0x0c(); inline void op_0x0d(); inline void op_0x0e(); inline void op_0x0f(); 
    inline void op_0x10(); inline void op_0x11(); inline void op_0x12(); inline void op_0x13(); inline void op_0x14(); inline void op_0x15(); inline void op_0x16(); inline void op_0x17(); 
    inline void op_0x18(); inline void op_0x19(); inline void op_0x1a(); inline void op_0x1b(); inline void op_0x1c(); inline void op_0x1d(); inline void op_0x1e(); inline void op_0x1f(); 
    inline void op_0x20(); inline void op_0x21(); inline void op_0x22(); inline void op_0x23(); inline void op_0x24(); inline void op_0x25(); inline void op_0x26(); inline void op_0x27(); 
    inline void op_0x28(); inline void op_0x29(); inline void op_0x2a(); inline void op_0x2b(); inline void op_0x2c(); inline void op_0x2d(); inline void op_0x2e(); inline void op_0x2f(); 
    inline void op_0x30(); inline void op_0x31(); inline void op_0x32(); inline void op_0x33(); inline void op_0x34(); inline void op_0x35(); inline void op_0x36(); inline void op_0x37(); 
    inline void op_0x38(); inline void op_0x39(); inline void op_0x3a(); inline void op_0x3b(); inline void op_0x3c(); inline void op_0x3d(); inline void op_0x3e(); inline void op_0x3f(); 
    inline void op_0x40(); inline void op_0x41(); inline void op_0x42(); inline void op_0x43(); inline void op_0x44(); inline void op_0x45(); inline void op_0x46(); inline void op_0x47(); 
    inline void op_0x48(); inline void op_0x49(); inline void op_0x4a(); inline void op_0x4b(); inline void op_0x4c(); inline void op_0x4d(); inline void op_0x4e(); inline void op_0x4f(); 
    inline void op_0x50(); inline void op_0x51(); inline void op_0x52(); inline void op_0x53(); inline void op_0x54(); inline void op_0x55(); inline void op_0x56(); inline void op_0x57(); 
    inline void op_0x58(); inline void op_0x59(); inline void op_0x5a(); inline void op_0x5b(); inline void op_0x5c(); inline void op_0x5d(); inline void op_0x5e(); inline void op_0x5f(); 
    inline void op_0x60(); inline void op_0x61(); inline void op_0x62(); inline void op_0x63(); inline void op_0x64(); inline void op_0x65(); inline void op_0x66(); inline void op_0x67(); 
    inline void op_0x68(); inline void op_0x69(); inline void op_0x6a(); inline void op_0x6b(); inline void op_0x6c(); inline void op_0x6d(); inline void op_0x6e(); inline void op_0x6f(); 
    inline void op_0x70(); inline void op_0x71(); inline void op_0x72(); inline void op_0x73(); inline void op_0x74(); inline void op_0x75(); inline void op_0x76(); inline void op_0x77(); 
    inline void op_0x78(); inline void op_0x79(); inline void op_0x7a(); inline void op_0x7b(); inline void op_0x7c(); inline void op_0x7d(); inline void op_0x7e(); inline void op_0x7f(); 
    inline void op_0x80(); inline void op_0x81(); inline void op_0x82(); inline void op_0x83(); inline void op_0x84(); inline void op_0x85(); inline void op_0x86(); inline void op_0x87(); 
    inline void op_0x88(); inline void op_0x89(); inline void op_0x8a(); inline void op_0x8b(); inline void op_0x8c(); inline void op_0x8d(); inline void op_0x8e(); inline void op_0x8f(); 
    inline void op_0x90(); inline void op_0x91(); inline void op_0x92(); inline void op_0x93(); inline void op_0x94(); inline void op_0x95(); inline void op_0x96(); inline void op_0x97(); 
    inline void op_0x98(); inline void op_0x99(); inline void op_0x9a(); inline void op_0x9b(); inline void op_0x9c(); inline void op_0x9d(); inline void op_0x9e(); inline void op_0x9f(); 
    inline void op_0xa0(); inline void op_0xa1(); inline void op_0xa2(); inline void op_0xa3(); inline void op_0xa4(); inline void op_0xa5(); inline void op_0xa6(); inline void op_0xa7(); 
    inline void op_0xa8(); inline void op_0xa9(); inline void op_0xaa(); inline void op_0xab(); inline void op_0xac(); inline void op_0xad(); inline void op_0xae(); inline void op_0xaf(); 
    inline void op_0xb0(); inline void op_0xb1(); inline void op_0xb2(); inline void op_0xb3(); inline void op_0xb4(); inline void op_0xb5(); inline void op_0xb6(); inline void op_0xb7(); 
    inline void op_0xb8(); inline void op_0xb9(); inline void op_0xba(); inline void op_0xbb(); inline void op_0xbc(); inline void op_0xbd(); inline void op_0xbe(); inline void op_0xbf(); 
    inline void op_0xc0(); inline void op_0xc1(); inline void op_0xc2(); inline void op_0xc3(); inline void op_0xc4(); inline void op_0xc5(); inline void op_0xc6(); inline void op_0xc7(); 
    inline void op_0xc8(); inline void op_0xc9(); inline void op_0xca(); inline void op_0xcb(); inline void op_0xcc(); inline void op_0xcd(); inline void op_0xce(); inline void op_0xcf(); 
    inline void op_0xd0(); inline void op_0xd1(); inline void op_0xd2(); inline void op_0xd3(); inline void op_0xd4(); inline void op_0xd5(); inline void op_0xd6(); inline void op_0xd7(); 
    inline void op_0xd8(); inline void op_0xd9(); inline void op_0xda(); inline void op_0xdb(); inline void op_0xdc(); inline void op_0xdd(); inline void op_0xde(); inline void op_0xdf(); 
    inline void op_0xe0(); inline void op_0xe1(); inline void op_0xe2(); inline void op_0xe3(); inline void op_0xe4(); inline void op_0xe5(); inline void op_0xe6(); inline void op_0xe7(); 
    inline void op_0xe8(); inline void op_0xe9(); inline void op_0xea(); inline void op_0xeb(); inline void op_0xec(); inline void op_0xed(); inline void op_0xee(); inline void op_0xef(); 
    inline void op_0xf0(); inline void op_0xf1(); inline void op_0xf2(); inline void op_0xf3(); inline void op_0xf4(); inline void op_0xf5(); inline void op_0xf6(); inline void op_0xf7(); 
    inline void op_0xf8(); inline void op_0xf9(); inline void op_0xfa(); inline void op_0xfb(); inline void op_0xfc(); inline void op_0xfd(); inline void op_0xfe(); inline void op_0xff(); 

    inline void op_cb_0x00(); inline void op_cb_0x01(); inline void op_cb_0x02(); inline void op_cb_0x03(); inline void op_cb_0x04(); inline void op_cb_0x05(); inline void op_cb_0x06(); inline void op_cb_0x07(); 
    inline void op_cb_0x08(); inline void op_cb_0x09(); inline void op_cb_0x0a(); inline void op_cb_0x0b(); inline void op_cb_0x0c(); inline void op_cb_0x0d(); inline void op_cb_0x0e(); inline void op_cb_0x0f(); 
    inline void op_cb_0x10(); inline void op_cb_0x11(); inline void op_cb_0x12(); inline void op_cb_0x13(); inline void op_cb_0x14(); inline void op_cb_0x15(); inline void op_cb_0x16(); inline void op_cb_0x17(); 
    inline void op_cb_0x18(); inline void op_cb_0x19(); inline void op_cb_0x1a(); inline void op_cb_0x1b(); inline void op_cb_0x1c(); inline void op_cb_0x1d(); inline void op_cb_0x1e(); inline void op_cb_0x1f(); 
    inline void op_cb_0x20(); inline void op_cb_0x21(); inline void op_cb_0x22(); inline void op_cb_0x23(); inline void op_cb_0x24(); inline void op_cb_0x25(); inline void op_cb_0x26(); inline void op_cb_0x27(); 
    inline void op_cb_0x28(); inline void op_cb_0x29(); inline void op_cb_0x2a(); inline void op_cb_0x2b(); inline void op_cb_0x2c(); inline void op_cb_0x2d(); inline void op_cb_0x2e(); inline void op_cb_0x2f(); 
    inline void op_cb_0x30(); inline void op_cb_0x31(); inline void op_cb_0x32(); inline void op_cb_0x33(); inline void op_cb_0x34(); inline void op_cb_0x35(); inline void op_cb_0x36(); inline void op_cb_0x37(); 
    inline void op_cb_0x38(); inline void op_cb_0x39(); inline void op_cb_0x3a(); inline void op_cb_0x3b(); inline void op_cb_0x3c(); inline void op_cb_0x3d(); inline void op_cb_0x3e(); inline void op_cb_0x3f(); 
    inline void op_cb_0x40(); inline void op_cb_0x41(); inline void op_cb_0x42(); inline void op_cb_0x43(); inline void op_cb_0x44(); inline void op_cb_0x45(); inline void op_cb_0x46(); inline void op_cb_0x47(); 
    inline void op_cb_0x48(); inline void op_cb_0x49(); inline void op_cb_0x4a(); inline void op_cb_0x4b(); inline void op_cb_0x4c(); inline void op_cb_0x4d(); inline void op_cb_0x4e(); inline void op_cb_0x4f(); 
    inline void op_cb_0x50(); inline void op_cb_0x51(); inline void op_cb_0x52(); inline void op_cb_0x53(); inline void op_cb_0x54(); inline void op_cb_0x55(); inline void op_cb_0x56(); inline void op_cb_0x57(); 
    inline void op_cb_0x58(); inline void op_cb_0x59(); inline void op_cb_0x5a(); inline void op_cb_0x5b(); inline void op_cb_0x5c(); inline void op_cb_0x5d(); inline void op_cb_0x5e(); inline void op_cb_0x5f(); 
    inline void op_cb_0x60(); inline void op_cb_0x61(); inline void op_cb_0x62(); inline void op_cb_0x63(); inline void op_cb_0x64(); inline void op_cb_0x65(); inline void op_cb_0x66(); inline void op_cb_0x67(); 
    inline void op_cb_0x68(); inline void op_cb_0x69(); inline void op_cb_0x6a(); inline void op_cb_0x6b(); inline void op_cb_0x6c(); inline void op_cb_0x6d(); inline void op_cb_0x6e(); inline void op_cb_0x6f(); 
    inline void op_cb_0x70(); inline void op_cb_0x71(); inline void op_cb_0x72(); inline void op_cb_0x73(); inline void op_cb_0x74(); inline void op_cb_0x75(); inline void op_cb_0x76(); inline void op_cb_0x77(); 
    inline void op_cb_0x78(); inline void op_cb_0x79(); inline void op_cb_0x7a(); inline void op_cb_0x7b(); inline void op_cb_0x7c(); inline void op_cb_0x7d(); inline void op_cb_0x7e(); inline void op_cb_0x7f(); 
    inline void op_cb_0x80(); inline void op_cb_0x81(); inline void op_cb_0x82(); inline void op_cb_0x83(); inline void op_cb_0x84(); inline void op_cb_0x85(); inline void op_cb_0x86(); inline void op_cb_0x87(); 
    inline void op_cb_0x88(); inline void op_cb_0x89(); inline void op_cb_0x8a(); inline void op_cb_0x8b(); inline void op_cb_0x8c(); inline void op_cb_0x8d(); inline void op_cb_0x8e(); inline void op_cb_0x8f(); 
    inline void op_cb_0x90(); inline void op_cb_0x91(); inline void op_cb_0x92(); inline void op_cb_0x93(); inline void op_cb_0x94(); inline void op_cb_0x95(); inline void op_cb_0x96(); inline void op_cb_0x97(); 
    inline void op_cb_0x98(); inline void op_cb_0x99(); inline void op_cb_0x9a(); inline void op_cb_0x9b(); inline void op_cb_0x9c(); inline void op_cb_0x9d(); inline void op_cb_0x9e(); inline void op_cb_0x9f(); 
    inline void op_cb_0xa0(); inline void op_cb_0xa1(); inline void op_cb_0xa2(); inline void op_cb_0xa3(); inline void op_cb_0xa4(); inline void op_cb_0xa5(); inline void op_cb_0xa6(); inline void op_cb_0xa7(); 
    inline void op_cb_0xa8(); inline void op_cb_0xa9(); inline void op_cb_0xaa(); inline void op_cb_0xab(); inline void op_cb_0xac(); inline void op_cb_0xad(); inline void op_cb_0xae(); inline void op_cb_0xaf(); 
    inline void op_cb_0xb0(); inline void op_cb_0xb1(); inline void op_cb_0xb2(); inline void op_cb_0xb3(); inline void op_cb_0xb4(); inline void op_cb_0xb5(); inline void op_cb_0xb6(); inline void op_cb_0xb7(); 
    inline void op_cb_0xb8(); inline void op_cb_0xb9(); inline void op_cb_0xba(); inline void op_cb_0xbb(); inline void op_cb_0xbc(); inline void op_cb_0xbd(); inline void op_cb_0xbe(); inline void op_cb_0xbf(); 
    inline void op_cb_0xc0(); inline void op_cb_0xc1(); inline void op_cb_0xc2(); inline void op_cb_0xc3(); inline void op_cb_0xc4(); inline void op_cb_0xc5(); inline void op_cb_0xc6(); inline void op_cb_0xc7(); 
    inline void op_cb_0xc8(); inline void op_cb_0xc9(); inline void op_cb_0xca(); inline void op_cb_0xcb(); inline void op_cb_0xcc(); inline void op_cb_0xcd(); inline void op_cb_0xce(); inline void op_cb_0xcf(); 
    inline void op_cb_0xd0(); inline void op_cb_0xd1(); inline void op_cb_0xd2(); inline void op_cb_0xd3(); inline void op_cb_0xd4(); inline void op_cb_0xd5(); inline void op_cb_0xd6(); inline void op_cb_0xd7(); 
    inline void op_cb_0xd8(); inline void op_cb_0xd9(); inline void op_cb_0xda(); inline void op_cb_0xdb(); inline void op_cb_0xdc(); inline void op_cb_0xdd(); inline void op_cb_0xde(); inline void op_cb_0xdf(); 
    inline void op_cb_0xe0(); inline void op_cb_0xe1(); inline void op_cb_0xe2(); inline void op_cb_0xe3(); inline void op_cb_0xe4(); inline void op_cb_0xe5(); inline void op_cb_0xe6(); inline void op_cb_0xe7(); 
    inline void op_cb_0xe8(); inline void op_cb_0xe9(); inline void op_cb_0xea(); inline void op_cb_0xeb(); inline void op_cb_0xec(); inline void op_cb_0xed(); inline void op_cb_0xee(); inline void op_cb_0xef(); 
    inline void op_cb_0xf0(); inline void op_cb_0xf1(); inline void op_cb_0xf2(); inline void op_cb_0xf3(); inline void op_cb_0xf4(); inline void op_cb_0xf5(); inline void op_cb_0xf6(); inline void op_cb_0xf7(); 
    inline void op_cb_0xf8(); inline void op_cb_0xf9(); inline void op_cb_0xfa(); inline void op_cb_0xfb(); inline void op_cb_0xfc(); inline void op_cb_0xfd(); inline void op_cb_0xfe(); inline void op_cb_0xff(); 
};

#endif
