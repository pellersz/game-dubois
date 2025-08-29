#ifndef CHANNELS_H
#define CHANNELS_H

#include "types.h"
#include <miniaudio.h>

enum ChannelType 
{
    Ch1,
    Ch2,
    Ch3,
    Ch4
};

class Channel 
{
public:
    bool dacOn = false;
    bool on = false;

    int stopTimer = 0;
    bool leftEnabled;
    bool rightEnabled;

    int time = 0;
    double advance;   
    float amplitude;

    virtual void setPeriodValue(unsigned short) = 0;
};

class SquareChannel: public Channel
{
public:
    constexpr static const byte DUTYS[] = { 0b11111110, 0b01111110, 0b01111000, 0b10000001 };
    u8 duty;
};

class Enveloped 
{
public:
    offs envelopeDir = 1;
};

class Channel1: public SquareChannel, public Enveloped 
{
public:
    virtual void setPeriodValue(unsigned short period) override;
};

class Channel2: public SquareChannel, public Enveloped 
{
public:
    virtual void setPeriodValue(unsigned short period) override;
};

class Channel3: public Channel
{
public:
    constexpr static const float VOLUME_LEVELS[] = { 0, 1, 0.5, 0.25 };
    float volumeLevel;
    virtual void setPeriodValue(unsigned short period) override;
};

class Channel4: public Channel, public Enveloped
{
public:
    unsigned short lfsr;

    virtual void setPeriodValue(unsigned short period) override;
};

class Channels 
{
public:
    Channel1 channel1;
    Channel2 channel2;
    Channel3 channel3;
    Channel4 channel4;
};


#endif
