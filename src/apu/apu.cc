#include "apu.h"
#include "channels.h"
#include "mem.h"
#include "speaker.h"

Apu::Apu(Memory& memory, Speaker& speaker): memory(memory), speaker(speaker) {}

void Apu::turnOnOffDac(ChannelType type, bool val)
{
    switch (type) 
    {
        case Ch1: 
        {
            speaker.channels.channel1.on = val; 
            break; 
        }
        case Ch2: 
        {
            speaker.channels.channel2.on = val;
            break; 
        }
        case Ch3: 
        {
            speaker.channels.channel3.on = val;
            break;
        }
        case Ch4: 
        {
            speaker.channels.channel4.on = val;
            break;
        }
    }

    byte& nr52 = memory[Memory::NR52];
    byte mask = 1 << type;
    nr52 = val ? (nr52 | mask) : (nr52 & ~mask);
}

void Apu::audioMasterChanged() 
{
    for (int i = 0; i < 4; ++i)
        turnOnOffDac((ChannelType)i, false);
}

// this would be much better with an array
void Apu::soundPanningChanged()
{
    byte panning = memory[Memory::NR51];
    speaker.channels.channel1.rightEnabled = panning & 0b00000001;
    speaker.channels.channel2.rightEnabled = panning & 0b00000010;
    speaker.channels.channel3.rightEnabled = panning & 0b00000100;
    speaker.channels.channel4.rightEnabled = panning & 0b00001000;
    speaker.channels.channel1.leftEnabled  = panning & 0b00010000;
    speaker.channels.channel2.leftEnabled  = panning & 0b00100000;
    speaker.channels.channel3.leftEnabled  = panning & 0b01000000;
    speaker.channels.channel4.leftEnabled  = panning & 0b10000000;
}

void Apu::envelope(ChannelType type)
{
    switch (type) 
    {
        case Ch1: { speaker.channels.channel1.envelope += speaker.channels.channel1.envelopeDir; break; }
        case Ch2: { speaker.channels.channel2.envelope += speaker.channels.channel2.envelopeDir; break; }
        case Ch3: break;
        case Ch4: { speaker.channels.channel4.envelope += speaker.channels.channel4.envelopeDir; break; }
    }
}

void Apu::nr11Changed() 
{
    byte nr11 = memory[Memory::NR11];
    speaker.channels.channel1.dutyRatio = SquareChannel::DUTY_RATIOS[(nr11 >> 6) & 0b11];
}

void Apu::nr12Changed()
{
    byte nr12 = memory[Memory::NR12];
    speaker.channels.channel1.envelopeDir = (nr12 & 0b00001000) ? -1 : 1;
    if (!(nr12 & 0b11111000))
        turnOnOffDac(Ch1, false);
}

void Apu::nr14Changed()
{
    byte nr14 = memory[Memory::NR14];
    speaker.channels.channel2.lengthEnabled = nr14 & 0b01000000;
    if (nr14 & 0b10000000)
    {
        speaker.channels.channel1.on = true;
        ch1Shadow = memory[Memory::NR13] + (memory[Memory::NR14] << 8);
        if (speaker.channels.channel1.stop_timer >= 64) 
            speaker.channels.channel1.stop_timer = memory[Memory::NR11] & 0b00111111;
        speaker.channels.channel1.envelope = 0;
        speaker.channels.channel1.amplitude = -1. + (memory[Memory::NR12] >> 4) * VOLUME_UNIT;

        byte nr10 = memory[Memory::NR10];
        u8 step = nr10 & 0b0111;
        u8 pace = (nr10 >> 4) & 0b0111;
    }
}

void Apu::nr21Changed()
{
    byte nr21 = memory[Memory::NR21];
    speaker.channels.channel2.dutyRatio = SquareChannel::DUTY_RATIOS[(nr21 >> 6) & 0b11];
}

void Apu::tickPeriod1(u8 val)
{
    unsigned short tmp = ch1Shadow;
    ch1Shadow += val;

    if ((ch1Shadow & 0x7ff) < (tmp & 0x7ff))
    {
        ch1Shadow = memory[Memory::NR13] + (memory[Memory::NR14] << 8);
        sample1();
    }
}

void Apu::tickPeriod2(u8 val)
{
    unsigned short tmp = ch2Shadow;
    ch2Shadow += val;

    if ((ch2Shadow & 0x7ff) < (tmp & 0x7ff))
    {
        ch2Shadow = memory[Memory::NR23] + (memory[Memory::NR24] << 8);
        sample2();
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

void Apu::sample1() {}

void Apu::sample2() {}

void Apu::sample3() {}

void Apu::sample() {}

