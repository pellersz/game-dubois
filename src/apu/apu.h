#ifndef APU_H
#define APU_H 

#include "channels.h"
#include "mem.h"
#include "speaker.h"

class Apu 
{
public:
    constexpr static const float VOLUME_UNIT = 1. / 7.5;
    constexpr static const float VLUME_UNIT  = 1. / 15;

    Apu(Memory&, Speaker&);
    
    void audioMasterChanged();
    void soundPanningChanged();
    void leftRightVolumeChanged();
    
    bool envelope(ChannelType);
    void incrementTimer(ChannelType);
    void tickPeriod(ChannelType, u8);

    bool ch1Sweep();
    
    void nr11Changed();
    void nr12Changed();
    void nr14Changed();

    void nr21Changed();
    void nr22Changed();
    void nr24Changed();

    void nr30Changed();
    void nr32Changed();
    void nr34Changed();

    void nr42Changed();
    void nr43Changed();
    void nr44Changed();

    void ch4Shift();

    float sample1();
    float sample2();
    float sample3();
    float sample4();
    void sample();

    bool isOn();

private:
    constexpr const static float SPEAKER_VOLUME_UNIT = (1. / (4 * 9.));
    constexpr static const float HPF_MULTIPLIER = 0.9930;

    Memory& memory;
    Speaker& speaker;
    Channels channels;

    bool on;
    float leftVolume   = 0.25;
    float rightVolume  = 0.25;
    float hpfBiasLeft  = 0.0;
    float hpfBiasRight = 0.0;

    unsigned short ch1Shadow;
    unsigned short ch2Shadow;
    unsigned short ch3Shadow;
    unsigned short ch4Shadow;

    void envelopedNrx2Changed(unsigned short nr_addr, Channel& channel, bool& envelope_dir);
    void turnOnOffDac(Channel&, bool);
};

#endif
