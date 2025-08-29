#include "apu.h"
#include "channels.h"
#include "mem.h"
#include "speaker.h"
#include <iomanip>
#include <iostream>
#include <unistd.h>

Apu::Apu(Memory& memory, Speaker& speaker): memory(memory), speaker(speaker) {}

void Apu::turnOnOffDac(ChannelType type, bool val)
{
    switch (type) 
    {
        case Ch1: 
        {
            Channel1& channel1 = channels.channel1;
            channel1.dacOn = val; 
            if (!val)
            {
                channel1.on = false;
                memory[Memory::NR52] &= 0b11111110;
            }
            break; 
        }
        case Ch2: 
        {
            Channel2& channel2 = channels.channel2;
            channel2.dacOn = val;
            if (!val)
            {
                channel2.on = false;
                memory[Memory::NR52] &= 0b11111101;
            }
            break; 
        }
        case Ch3: 
        {
            Channel3& channel3 = channels.channel3;
            channel3.dacOn = val;
            if (!val)
            {
                channel3.on = false;
                memory[Memory::NR52] &= 0b11111011;
            }
            break;
        }
        case Ch4: 
        {
            Channel4& channel4 = channels.channel4;
            channel4.dacOn = val;
            if (!val)
            {
                channel4.on = false;
                memory[Memory::NR52] &= 0b11110111;
            }
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
            Channel1& channel1 = channels.channel1;
            float amplitude = channel1.amplitude; 
            amplitude += channel1.envelopeDir ? -VLUME_UNIT : VLUME_UNIT; 
            if ((amplitude < 0) || (amplitude > 1))
                return false; 
            channel1.amplitude = amplitude;
            return true;
        }
        case Ch2: 
        { 
            Channel2& channel2 = channels.channel2;
            float amplitude = channel2.amplitude; 
            amplitude += channel2.envelopeDir ? -VLUME_UNIT : VLUME_UNIT; 
            if ((amplitude < 0) || (amplitude > 1))
                return false; 
            channel2.amplitude = amplitude;
            return true;
        }        
        case Ch3: return false;
        case Ch4: 
        { 
            Channel4& channel4 = channels.channel4;
            float amplitude = channel4.amplitude; 
            amplitude += channel4.envelopeDir ? -VLUME_UNIT : VLUME_UNIT; 
            if ((amplitude < 0) || (amplitude > 1))
                return false; 
            channel4.amplitude = amplitude;
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
            if (++channels.channel1.stopTimer >= 0b01000000)
                channels.channel1.on = false;
            break; 
        }
        case Ch2: 
        { 
            if (++channels.channel2.stopTimer >= 0b01000000)
                channels.channel2.on = false;
            break; 
        }
        case Ch3: 
        { 
            if (++channels.channel3.stopTimer >= 0x100)
                channels.channel3.on = false;
            break; 
        }
        case Ch4: 
        { 
            if (++channels.channel4.stopTimer == 0b01000000)
                channels.channel4.on = false;
            break; 
        }
    }
}

void Apu::nr11Changed() { channels.channel1.duty = SquareChannel::DUTYS[memory[Memory::NR11] >> 6]; }

void Apu::nr12Changed()
{
    byte nr12 = memory[Memory::NR12];
    Channel1& channel1 = channels.channel1;
    channel1.envelopeDir = (nr12 & 0b00001000) ? -1 : 1;
    bool new_dac_on = (nr12 & 0b11111000);
    if (channel1.dacOn != new_dac_on)
        turnOnOffDac(Ch1, new_dac_on);
}

void Apu::nr14Changed()
{
    if (memory[Memory::NR14] & 0b10000000)
    {
        Channel1& channel1 = channels.channel1;
        channel1.on = channel1.dacOn;
        memory[Memory::NR52] |= 0b0001;
        ch1Shadow = memory[Memory::NR13] + (memory[Memory::NR14] << 8);
        if (channel1.stopTimer >= 0b01000000) 
            channel1.stopTimer = memory[Memory::NR11] & 0b00111111;
        channel1.amplitude = (memory[Memory::NR12] >> 4) * VLUME_UNIT;
    }
}

void Apu::nr21Changed() { channels.channel2.duty = SquareChannel::DUTYS[memory[Memory::NR21] >> 6]; }

void Apu::nr22Changed() 
{ 
    byte nr22 = memory[Memory::NR22];
    Channel2& channel2 = channels.channel2;
    channel2.envelopeDir = (nr22 & 0b00001000) ? -1 : 1;
    bool new_dac_on = (nr22 & 0b11111000);
    if (channel2.dacOn != new_dac_on)
        turnOnOffDac(Ch2, new_dac_on);
}

void Apu::nr24Changed()
{
    if (memory[Memory::NR24] & 0b10000000)
    {
        Channel2& channel2 = channels.channel2;
        channel2.on = channel2.dacOn;
        memory[Memory::NR52] |= 0b0010;
        ch2Shadow = memory[Memory::NR23] + (memory[Memory::NR24] << 8);
        if (channel2.stopTimer >= 0b01000000) 
            channel2.stopTimer = memory[Memory::NR21] & 0b00111111;
        channel2.amplitude = (memory[Memory::NR22] >> 4) * VLUME_UNIT;
    }
}

void Apu::nr30Changed() 
{ 
    bool new_dac_on = (memory[Memory::NR30] & 0b10000000);
    if (channels.channel3.dacOn != new_dac_on)
        turnOnOffDac(Ch3, new_dac_on);
}

void Apu::nr32Changed()
{
    channels.channel3.volumeLevel = Channel3::VOLUME_LEVELS[(memory[Memory::NR32] >> 5) & 0b0011];
}

void Apu::nr34Changed()
{
    if (memory[Memory::NR34] & 0b10000000)
    {
        Channel3& channel3 = channels.channel3;
        channel3.on = channel3.dacOn;
        memory[Memory::NR52] |= 0b0100;
        ch3Shadow = memory[Memory::NR33] + (memory[Memory::NR34] << 8);
        channel3.time = 0;
        if (channel3.stopTimer >= 0x100) 
            channel3.stopTimer = memory[Memory::NR31] & 0b00111111;
        channel3.amplitude = (memory[Memory::NR32] >> 4) * VLUME_UNIT;
    }
}

void Apu::nr42Changed()
{
    byte nr42 = memory[Memory::NR42];
    Channel4& channel4 = channels.channel4;
    channel4.envelopeDir = (nr42 & 0b00001000) ? -1 : 1;
    bool new_dac_on = (nr42 & 0b11111000);
    if (channel4.dacOn != new_dac_on)
        turnOnOffDac(Ch4, new_dac_on);   
}

void Apu::nr43Changed() 
{
    memory[Memory::NR43] & 0b1000;
}

void Apu::nr44Changed()
{
    if (memory[Memory::NR44] & 0b10000000)
    {
        Channel4& channel4 = channels.channel4;
        channel4.on = channel4.dacOn;
        channel4.lfsr = 0;
        memory[Memory::NR52] |= 0b1000;
        if (channel4.stopTimer >= 0b01000000) 
            channel4.stopTimer = memory[Memory::NR41] & 0b00111111;
        channel4.amplitude = (memory[Memory::NR42] >> 4) * VLUME_UNIT;
    }
}

void Apu::tickPeriod1(u8 val)
{
    unsigned short tmp = ch1Shadow;
    ch1Shadow += val;

    if ((ch1Shadow & 0x7ff) < (tmp & 0x7ff))
    {
        ch1Shadow = memory[Memory::NR13] + (memory[Memory::NR14] << 8);
        if (++channels.channel1.time >= 8)
            channels.channel1.time = 0;
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
    unsigned short tmp = ch3Shadow;
    ch3Shadow += val;

    if ((ch3Shadow & 0x7ff) < (tmp & 0x7ff))
    {
        ch3Shadow = memory[Memory::NR33] + (memory[Memory::NR34] << 8);
        if (++channels.channel3.time >= 32)
            channels.channel3.time = 0;
    }
}

void Apu::ch4Shift()
{
    unsigned short& lfsr = channels.channel4.lfsr;
    lfsr >>= 1;
    bool next = ((lfsr & 0b0001) == ((lfsr >> 1) & 0b0001));
    lfsr |= (next << 15);
    if (memory[Memory::NR43] & 0b00001000)
        lfsr |= (next << 7);
}

float Apu::sample1() 
{
    Channel1& channel1 = channels.channel1;
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
    Channel2& channel2 = channels.channel2;
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
    Channel3& channel3 = channels.channel3;
    if (channel3.dacOn) 
    {
        if (channel3.on) 
        {
            u8 val = memory[Memory::WAVE_PATTERN + channel3.time / 2];
            if (!(channel3.time % 2))
                val >>= 4;
            else
                val &= 0b1111;
            
            return channel3.volumeLevel * (1 - val * VOLUME_UNIT); 
        }
        return 1.0;
    }
    return 0;
}

float Apu::sample4() 
{
    Channel4& channel4 = channels.channel4;
    if (channel4.dacOn) 
    {
        if (channel4.on) 
            return (channel4.lfsr & 0b0001) ? channel4.amplitude : 1.0; 
        return 1.0;
    }
    return 0;
}

void Apu::sample() 
{
    float s1 = sample1(), s2 = sample2(), s3 = sample3(), s4 = sample4();   
    
    //if (s1 > 0.6)
    //    std::cout << s1 << " ";

    float unfiltered_left = 
        (channels.channel1.leftEnabled ? s1 : 0) +
        (channels.channel2.leftEnabled ? s2 : 0) +
        (channels.channel3.leftEnabled ? s3 : 0) +
        (channels.channel4.leftEnabled ? s4 : 0)
    ;
    unfiltered_left *= leftVolume;
    float left = unfiltered_left - hpfBiasLeft;

    float unfiltered_right = 
        (channels.channel1.rightEnabled ? s1 : 0) +
        (channels.channel2.rightEnabled ? s2 : 0) +
        (channels.channel3.rightEnabled ? s3 : 0) +
        (channels.channel4.rightEnabled ? s4 : 0)
    ;
    unfiltered_right *= rightVolume;
    float right = unfiltered_right - hpfBiasRight;

    hpfBiasLeft  = unfiltered_left  - HPF_MULTIPLIER * left;
    hpfBiasRight = unfiltered_right - HPF_MULTIPLIER * right;

    speaker.sampleBuffer.sample(left, right);
}

bool Apu::isOn() { return on; }
