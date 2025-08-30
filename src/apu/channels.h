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
    bool on    = false;

    int stopTimer = 0;

    bool leftEnabled  = 0;
    bool rightEnabled = 0;

    int progress    = 0;
    float amplitude = 0;

    u8 type;
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
    bool envelopeDir = 1;
};

class Channel1: public SquareChannel, public Enveloped 
{ 
public:
    Channel1() { type = 1; }
};

class Channel2: public SquareChannel, public Enveloped 
{
public:
    Channel2() { type = 2; }
};

class Channel3: public Channel
{
public:
    Channel3() { type = 3; }
    constexpr static const float VOLUME_LEVELS[] = { 0, 1, 0.5, 0.25 };
    float volumeLevel;
};

class Channel4: public Channel, public Enveloped
{
public:
    Channel4() { type = 4; }
    unsigned short lfsr = 0;
    bool bit_8 = 0;
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
