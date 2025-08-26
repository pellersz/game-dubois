#include "apu.h"
#include "channels.h"
#include "mem.h"
#include "speaker.h"
#include <iostream>

Apu::Apu(Memory& memory, Speaker& speaker): memory(memory), speaker(speaker) {}

void Apu::turnOnOffDac(ChannelType type, bool val)
{
    switch (type) 
    {
        case Ch1: 
        {
            channels.channel1.dacOn = val; 
                channels.channel1.on = channels.channel1.on && val; 
            break; 
        }
        case Ch2: 
        {
            channels.channel2.dacOn = val;
            channels.channel2.on = channels.channel2.on && val; 
            break; 
        }
        case Ch3: 
        {
            channels.channel3.dacOn = val;
            channels.channel3.on = channels.channel3.on && val; 
            break;
        }
        case Ch4: 
        {
            channels.channel4.dacOn = val;
            channels.channel4.on = channels.channel4.on && val; 
            break;
        }
    }

    byte& nr52 = memory[Memory::NR52];
    byte mask = 1 << type;
    nr52 = val ? (nr52 | mask) : (nr52 & ~mask);
}

void Apu::audioMasterChanged() 
{
    if (!(memory[Memory::NR52] & 0b10000000))
    {
        on = false;
        for (int i = 0; i < 4; ++i)
            turnOnOffDac((ChannelType)i, false);
    } 
    else 
        on = true;
}

// this would be much better with an array
void Apu::soundPanningChanged()
{
    byte panning = memory[Memory::NR51];
    channels.channel1.rightEnabled = panning & 0b00000001;
    channels.channel2.rightEnabled = panning & 0b00000010;
    channels.channel3.rightEnabled = panning & 0b00000100;
    channels.channel4.rightEnabled = panning & 0b00001000;
    channels.channel1.leftEnabled  = panning & 0b00010000;
    channels.channel2.leftEnabled  = panning & 0b00100000;
    channels.channel3.leftEnabled  = panning & 0b01000000;
    channels.channel4.leftEnabled  = panning & 0b10000000;
}

void Apu::leftRightVolumeChanged()
{
    byte nr50 = memory[Memory::NR50];
    // TODO: constants
    leftVolume  = (1. / (4 * 9.)) * (((nr50 >> 4) & 0b0111) + 1);
    rightVolume = (1. / (4 * 9.)) * ((nr50 & 0b0111) + 1);
}

// TODO: this should be handled better
bool Apu::envelope(ChannelType type)
{
    switch (type) 
    {
        case Ch1: 
        { 
            float amplitude = channels.channel1.amplitude; 
            amplitude += channels.channel1.envelopeDir ? VOLUME_UNIT : -VOLUME_UNIT; 
            if ((amplitude < -1.0) || (amplitude > 1.0))
                return false; 
            channels.channel1.amplitude = amplitude;
            return true;
        }
        case Ch2: 
        { 
            float amplitude = channels.channel2.amplitude; 
            amplitude += channels.channel2.envelopeDir ? VOLUME_UNIT : -VOLUME_UNIT; 
            if ((amplitude < -1.0) || (amplitude > 1.0))
                return false; 
            channels.channel2.amplitude = amplitude;
            return true;
        }        
        case Ch3: return false;
        case Ch4: 
        { 
            float amplitude = channels.channel4.amplitude; 
            amplitude += channels.channel4.envelopeDir ? VOLUME_UNIT : -VOLUME_UNIT; 
            if ((amplitude < -1.0) || (amplitude > 1.0))
                return false; 
            channels.channel4.amplitude = amplitude;
            return true;
        }
    }

    return false;
}

void Apu::incrementTimer(ChannelType type) 
{
    switch (type) 
    {
        case Ch1: 
        { 
            if (++channels.channel1.stopTimer == 64)
                channels.channel1.on = false;
            break; 
        }
        case Ch2: 
        { 
            if (++channels.channel1.stopTimer == 64)
                channels.channel1.on = false;
            break; 
        }
        case Ch3: 
        { 
            if (++channels.channel1.stopTimer == 64)
                channels.channel1.on = false;
            break; 
        }
        case Ch4: 
        { 
            if (++channels.channel1.stopTimer == 64)
                channels.channel1.on = false;
            break; 
        }
    }
}

void Apu::nr11Changed() { channels.channel1.duty = SquareChannel::DUTYS[memory[Memory::NR11] >> 6]; }

void Apu::nr12Changed()
{
    byte nr12 = memory[Memory::NR12];
    channels.channel1.envelopeDir = (nr12 & 0b00001000) ? -1 : 1;
    if (!(nr12 & 0b11111000))
        turnOnOffDac(Ch1, false);
}

void Apu::nr14Changed()
{
    if (memory[Memory::NR14] & 0b10000000)
    {
        channels.channel1.on = channels.channel1.dacOn;
        ch1Shadow = memory[Memory::NR13] + (memory[Memory::NR14] << 8);
        if (channels.channel1.stopTimer >= 64) 
            channels.channel1.stopTimer = memory[Memory::NR11] & 0b00111111;
        channels.channel1.amplitude = 1 - (memory[Memory::NR12] >> 4) * VOLUME_UNIT;
    }
}

void Apu::nr21Changed() { channels.channel2.duty = SquareChannel::DUTYS[memory[Memory::NR21] >> 6]; }

void Apu::tickPeriod1(u8 val)
{
    unsigned short tmp = ch1Shadow;
    ch1Shadow += val;

    if ((ch1Shadow & 0x7ff) < (tmp & 0x7ff))
    {
        ch1Shadow = memory[Memory::NR13] + (memory[Memory::NR14] << 8);
        if (++channels.channel1.time >= 8)
            channels.channel2.time = 0;
    }
}

void Apu::tickPeriod2(u8 val)
{
    unsigned short tmp = ch2Shadow;
    ch2Shadow += val;

    if ((ch2Shadow & 0x7ff) < (tmp & 0x7ff))
    {
        ch2Shadow = memory[Memory::NR23] + (memory[Memory::NR24] << 8);
        if (++channels.channel2.time >= 8)
            channels.channel2.time = 0;
    }
}

void Apu::tickPeriod3(u8 val)
{
    unsigned short tmp = ch1Shadow;
    ch1Shadow += val;

    if ((ch1Shadow & 0x7ff) < (tmp & 0x7ff))
    {
        ch1Shadow = memory[Memory::NR33] + (memory[Memory::NR34] << 8);
        sample3();
    }
}

float Apu::sample1() 
{
    Channel1 channel1 = channels.channel1;
    if (channel1.dacOn) 
    {
        if (channel1.on) 
            return ((channel1.duty << channel1.time) & 0b10000000) ? channel1.amplitude : -channel1.amplitude; 
        return 1.0;
    }
    return 0;
}

float Apu::sample2() 
{
    Channel1 channel2 = channels.channel1;
    if (channel2.dacOn) 
    {
        if (channel2.on) 
            return ((channel2.duty << channel2.time) & 0b10000000) ? channel2.amplitude : -channel2.amplitude; 
        return 1.0;
    }
    return 0;
}

float Apu::sample3() 
{
    return 0;
}

float Apu::sample4() 
{
    return 0;
}

void Apu::sample() 
{
    float s1 = sample1(), s2 = sample2(), s3 = sample3(), s4 = sample4();   
    float left = 
        (channels.channel1.leftEnabled ? s1 : 0) +
        (channels.channel2.leftEnabled ? s2 : 0) +
        (channels.channel3.leftEnabled ? s3 : 0) +
        (channels.channel1.leftEnabled ? s4 : 0)
    ;

    left *= leftVolume;

    float right = 
        (channels.channel1.rightEnabled ? s1 : 0) +
        (channels.channel2.rightEnabled ? s2 : 0) +
        (channels.channel3.rightEnabled ? s3 : 0) +
        (channels.channel1.rightEnabled ? s4 : 0)
    ;
    right *= rightVolume;

    speaker.sampleBuffer.sample(left, right);
}

bool Apu::isOn() { return on; }
