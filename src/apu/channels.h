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
    bool on;

    u8 stop_timer = 0;
    bool leftEnabled;
    bool rightEnabled;
    bool lengthEnabled;

    double time = 0;
    double advance;   
    float amplitude;

    virtual void setPeriodValue(unsigned short) = 0;
};

class SquareChannel: public Channel
{
public:
    float dutyRatio;
    constexpr static const float DUTY_RATIOS[] = { 0.125, 0.25, 0.5, 1.0 };
};

class Enveloped 
{
public:
    u8 envelope = 0;
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
    u8 envelope = 0;
    virtual void setPeriodValue(unsigned short period) override;
};

class Channel3: public Channel
{
public:
    virtual void setPeriodValue(unsigned short period) override;
};

class Channel4: public Channel, public Enveloped
{
public:
    u8 envelope = 0;

    virtual void setPeriodValue(unsigned short period) override;
};

class Channels 
{
public:
    ma_data_source_base base;
    Channel1 channel1;
    Channel2 channel2;
    Channel3 channel3;
    Channel4 channel4;
};


#endif
