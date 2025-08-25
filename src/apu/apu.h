#ifndef APU_H
#define APU_H 

#include "channels.h"
#include "mem.h"
#include "speaker.h"

class Apu 
{
public:
    constexpr static const float VOLUME_UNIT = 1. / 8.;

    Apu(Memory&, Speaker&);
    
    void turnOnOffDac(ChannelType, bool);
    void audioMasterChanged();
    void soundPanningChanged();
    
    void envelope(ChannelType);
    
    void nr11Changed();
    void nr12Changed();
    void nr14Changed();

    void nr21Changed();

    void tickPeriod1(u8);
    void tickPeriod2(u8);
    void tickPeriod3(u8);

    void sample1();
    void sample2();
    void sample3();
    void sample();

private:
    Memory& memory;
    Speaker& speaker;

    unsigned short ch1Shadow;
    unsigned short ch2Shadow;
    unsigned short ch3Shadow;
    unsigned short ch4Shadow;
};

#endif
