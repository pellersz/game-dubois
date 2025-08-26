#ifndef APU_H
#define APU_H 

#include "channels.h"
#include "mem.h"
#include "speaker.h"

class Apu 
{
public:
    constexpr static const float VOLUME_UNIT = 1. / 7.5;

    Apu(Memory&, Speaker&);
    
    void turnOnOffDac(ChannelType, bool);
    void audioMasterChanged();
    void soundPanningChanged();
    void leftRightVolumeChanged();
    
    bool envelope(ChannelType);
    void incrementTimer(ChannelType);
    
    void nr11Changed();
    void nr12Changed();
    void nr14Changed();

    void nr21Changed();

    void tickPeriod1(u8);
    void tickPeriod2(u8);
    void tickPeriod3(u8);

    float sample1();
    float sample2();
    float sample3();
    float sample4();
    void sample();

    bool isOn();

private:
    Memory& memory;
    Speaker& speaker;
    Channels channels;
    bool on;
    float leftVolume = 0.25;
    float rightVolume = 0.25;

    unsigned short ch1Shadow;
    unsigned short ch2Shadow;
    unsigned short ch3Shadow;
    unsigned short ch4Shadow;
};

#endif
