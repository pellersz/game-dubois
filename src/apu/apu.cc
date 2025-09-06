#include "apu.h"
#include "channels.h"
#include "mem.h"
#include "speaker.h"
#include <unistd.h>

Apu::Apu(Memory& memory, Speaker& speaker): 
    memory(memory), 
    speaker(speaker),
    nr52(memory.buildIn(Memory::NR52)),
    nr51(memory.buildIn(Memory::NR51)),
    nr50(memory.buildIn(Memory::NR50)),
    nr10(memory.buildIn(Memory::NR10)),
    nr11(memory.buildIn(Memory::NR11)),
    nr12(memory.buildIn(Memory::NR12)),
    nr13(memory.buildIn(Memory::NR13)),
    nr14(memory.buildIn(Memory::NR14)),    
    nr21(memory.buildIn(Memory::NR21)),
    nr22(memory.buildIn(Memory::NR22)),
    nr23(memory.buildIn(Memory::NR23)),
    nr24(memory.buildIn(Memory::NR24)),
    nr30(memory.buildIn(Memory::NR30)),
    nr31(memory.buildIn(Memory::NR31)),
    nr32(memory.buildIn(Memory::NR32)),
    nr33(memory.buildIn(Memory::NR33)),
    nr34(memory.buildIn(Memory::NR34)),    
    nr41(memory.buildIn(Memory::NR41)),
    nr42(memory.buildIn(Memory::NR42)),
    nr43(memory.buildIn(Memory::NR43)),
    nr44(memory.buildIn(Memory::NR44))    
{}

void Apu::turnOnOffDac(Channel& channel, bool val)
{
    channel.dacOn = val; 
    if (!val) channel.on = false;

    byte mask = 1 << (channel.type - 1);
    nr52 = val ? (nr52 | mask) : (nr52 & ~mask);
}

void Apu::audioMasterChanged() 
{
    if (!(nr52 & 0b10000000))
    {
        on = false;
        turnOnOffDac(channels.channel1, false);
        turnOnOffDac(channels.channel2, false);
        turnOnOffDac(channels.channel3, false);
        turnOnOffDac(channels.channel4, false);
    } 
    else 
        on = true;
}

void Apu::soundPanningChanged()
{
    channels.channel1.rightEnabled = nr51 & 0b00000001;
    channels.channel2.rightEnabled = nr51 & 0b00000010;
    channels.channel3.rightEnabled = nr51 & 0b00000100;
    channels.channel4.rightEnabled = nr51 & 0b00001000;
    channels.channel1.leftEnabled  = nr51 & 0b00010000;
    channels.channel2.leftEnabled  = nr51 & 0b00100000;
    channels.channel3.leftEnabled  = nr51 & 0b01000000;
    channels.channel4.leftEnabled  = nr51 & 0b10000000;
}

void Apu::leftRightVolumeChanged()
{
    leftVolume  = SPEAKER_VOLUME_UNIT * (((nr50 >> 4) & 0b0111) + 1);
    rightVolume = SPEAKER_VOLUME_UNIT * ((nr50 & 0b0111) + 1);
}

bool Apu::envelope(ChannelType type)
{
    float* old_amplitude;
    bool envelopeDir;
    switch (type) 
    {
        case Ch1: 
        { 
            old_amplitude = &channels.channel1.amplitude; 
            envelopeDir = channels.channel1.envelopeDir;
            break;
        }
        case Ch2: 
        { 
            old_amplitude = &channels.channel2.amplitude; 
            envelopeDir = channels.channel2.envelopeDir;
            break;
        }        
        case Ch3: return false;
        case Ch4: 
        { 
            old_amplitude = &channels.channel4.amplitude; 
            envelopeDir = channels.channel4.envelopeDir;
            break;
        }
    }

    float new_amplitude = *old_amplitude;
    new_amplitude += envelopeDir ? -VLUME_UNIT : VLUME_UNIT; 
    if ((new_amplitude < 0) || (new_amplitude > 1))
        return false; 
    *old_amplitude = new_amplitude;
    return true;
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

void Apu::tickPeriod(ChannelType type, u8 val)
{
    unsigned short* ch_shadow;
    Channel* channel;
    unsigned short nr_addr;
    u8 period = 8;
    
    switch (type) {
        case Ch1:
        {
            ch_shadow = &ch1Shadow;
            channel = &channels.channel1;
            nr_addr = Memory::NR13;
            break;
        }
        case Ch2:
        {
            ch_shadow = &ch2Shadow;
            channel = &channels.channel2;
            nr_addr = Memory::NR23;
            break;
        }
        case Ch3:
        {
            ch_shadow = &ch3Shadow;
            channel = &channels.channel3;
            nr_addr = Memory::NR33;
            period = 32;
            break;
        }
        case Ch4:
            return;
    }

    unsigned short tmp = *ch_shadow;
    *ch_shadow += val;

    if ((*ch_shadow & 0x7ff) < (tmp & 0x7ff))
    {
        *ch_shadow = memory.read(nr_addr) + (memory.read(nr_addr + 1) << 8);
        if (++(channel->progress) >= period)
            channel->progress = 0;
    }
}

void Apu::envelopedNrx2Changed(unsigned short nr_addr, Channel& channel, bool& envelope_dir) 
{
    byte nr = memory.read(nr_addr);
    envelope_dir = (nr & 0b00001000) ? -1 : 1;
    bool new_dac_on = (nr & 0b11111000);
    if (channel.dacOn != new_dac_on)
        turnOnOffDac(channel, new_dac_on);   
}

bool Apu::ch1Sweep() 
{
    bool direction = nr10 & 0b00001000;
    u8 step = nr10 & 0b0111;

    unsigned short old_period = nr13 + ((nr14 & 0b0111) << 8); 
    unsigned short offs = old_period >> step;
    unsigned short new_period; 

    if (direction) 
        new_period = old_period - offs;
    else 
    {
        new_period = old_period + offs;  
        if (new_period > 0x07ff)
        {
            turnOnOffDac(channels.channel1, false);
            return false;
        }
    }

    nr13 = new_period;
    nr14 = ((new_period & 0x0700) >> 8) + (nr14 & 0b11111000);
    return true;
}

void Apu::nr11Changed() { channels.channel1.duty = SquareChannel::DUTYS[nr11 >> 6]; }

void Apu::nr12Changed()
{
    envelopedNrx2Changed(Memory::NR12, channels.channel1, channels.channel1.envelopeDir);
}

void Apu::nr14Changed()
{
  if (nr14 & 0b10000000) {
    Channel1 &channel1 = channels.channel1;
    channel1.on = channel1.dacOn;
    nr52 |= 0b0001;
    ch1Shadow = nr13 + (nr14 << 8);
    if (channel1.stopTimer >= 0b01000000)
      channel1.stopTimer = nr11 & 0b00111111;
    channel1.amplitude = (nr12 >> 4) * VLUME_UNIT;
  }
}

void Apu::nr21Changed() { channels.channel2.duty = SquareChannel::DUTYS[nr21 >> 6]; }

void Apu::nr22Changed() 
{ 
    envelopedNrx2Changed(Memory::NR22, channels.channel2, channels.channel2.envelopeDir);
}

void Apu::nr24Changed()
{
    if (nr24 & 0b10000000)
    {
        Channel2& channel2 = channels.channel2;
        channel2.on = channel2.dacOn;
        nr52 |= 0b0010;
        ch2Shadow = nr23 + (nr24 << 8);
        if (channel2.stopTimer >= 0b01000000) 
            channel2.stopTimer = nr21 & 0b00111111;
        channel2.amplitude = (nr22 >> 4) * VLUME_UNIT;
    }
}

void Apu::nr30Changed() 
{ 
    bool new_dac_on = (nr30 & 0b10000000);
    Channel3& channel3 = channels.channel3;
    if (channel3.dacOn != new_dac_on)
        turnOnOffDac(channel3, new_dac_on);
}

void Apu::nr32Changed()
{
    channels.channel3.volumeLevel = Channel3::VOLUME_LEVELS[(nr32 >> 5) & 0b0011];
}

void Apu::nr34Changed()
{
    if (nr34 & 0b10000000)
    {
        Channel3& channel3 = channels.channel3;
        channel3.on = channel3.dacOn;
        nr52 |= 0b0100;
        ch3Shadow = nr33 + (nr34 << 8);
        channel3.progress = 0;
        if (channel3.stopTimer >= 0x100) 
            channel3.stopTimer = nr31 & 0b00111111;
        channel3.amplitude = (nr32 >> 4) * VLUME_UNIT;
    }
}

void Apu::nr42Changed()
{
    envelopedNrx2Changed(Memory::NR42, channels.channel4, channels.channel4.envelopeDir);
}

void Apu::nr43Changed() { channels.channel4.bit_8 = nr43 & 0b1000; }

void Apu::nr44Changed()
{
    if (nr44 & 0b10000000)
    {
        Channel4& channel4 = channels.channel4;
        channel4.on = channel4.dacOn;
        channel4.lfsr = 0;
        nr52 |= 0b1000;
        if (channel4.stopTimer >= 0b01000000) 
            channel4.stopTimer = nr41 & 0b00111111;
        channel4.amplitude = (nr42 >> 4) * VLUME_UNIT;
    }
}

void Apu::ch4Shift()
{
    Channel4& channel4 = channels.channel4;
    unsigned short& lfsr = channel4.lfsr;

    bool next = ((lfsr & 0b0001) == ((lfsr >> 1) & 0b0001));

    lfsr |= (next << 15);
    if (channel4.bit_8)
    {
        if (next != (bool)(lfsr & 0b10000000))
        {
            if (next)
                lfsr |= 0b10000000;
            else 
                lfsr &= 0b01111111;
        }
    }
    lfsr >>= 1;
}

float Apu::sample1() 
{
    Channel1& channel1 = channels.channel1;
    if (channel1.dacOn) 
    {
        if (channel1.on) 
            return ((channel1.duty << channel1.progress) & 0b10000000) ? channel1.amplitude : -channel1.amplitude;
    }
    return 0;
}

float Apu::sample2() 
{
    Channel2& channel2 = channels.channel2;
    if (channel2.dacOn) 
    {
        if (channel2.on) 
            return ((channel2.duty << channel2.progress) & 0b10000000) ? channel2.amplitude : -channel2.amplitude; 
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
            u8 val = memory.read(Memory::WAVE_PATTERN + channel3.progress / 2);
            if (!(channel3.progress % 2))
                val >>= 4;
            else
                val &= 0b1111;
            
            return channel3.volumeLevel * (1 - val * VOLUME_UNIT); 
        }
    }
    return 0;
}

float Apu::sample4() 
{
    Channel4& channel4 = channels.channel4;
    if (channel4.dacOn) 
    {
        if (channel4.on) 
            return (channel4.lfsr & 0b0001) ? channel4.amplitude : -channel4.amplitude; 
    }
    return 0;
}

void Apu::sample() 
{
    float s1 = sample1(), s2 = sample2(), s3 = sample3(), s4 = sample4();   

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

