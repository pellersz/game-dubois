#include "mem.h"
#include "scheduler.h"
#include "apu.h"
#include "types.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <unistd.h>
#include "cartridge.h"

void Memory::init
(
    Scheduler *p_scheduler,
    Apu *p_apu
) 
{
    pScheduler = p_scheduler;
    pApu = p_apu;
}

void Memory::load(std::shared_ptr<Cartridge> p_cartridge) { pCartridge = p_cartridge; }

byte Memory::read(unsigned short addr)
{
    auto access_cartridge = pCartridge.lock();
    if (!access_cartridge)
    {
        std::cout << "Cartridge pointer expired" << std::endl;
        return 0xff;
    }
    //if (addr == LCD_Y)
    //    return 0x90;
    switch (addr >> 12)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            return access_cartridge->readBank(addr);
        case 4:
        case 5:
        case 6:
        case 7:
            return access_cartridge->readBankN(addr - 0x4000);
        case 8:
        case 9:
            return videoRam[addr - 0x8000];
        case 10:
        case 11:
            return access_cartridge->readRam(addr - 0xa000);
        case 12:
        case 13:
            return workRam[addr - 0xc000];
        case 14:
            return 0xff;
        case 15:
        {
            if (addr < 0xfe00)
                return 0xff;
            else if (addr < 0xfea0)
                return oam[addr - 0xfe00];
            else if (addr < 0xff00)
                return 0xff;
            else 
                return last0x100[addr - 0xff00];
        }
        default:
            return 0xff;
    }
}

void Memory::write(unsigned short addr, byte val)
{
    auto access_cartridge = pCartridge.lock();
    if (!access_cartridge)
    {
        std::cout << "Cartridge pointer expired" << std::endl;
        return;
    }

    switch (addr >> 12)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        {
            access_cartridge->writeToRegister(addr, val);
            return;
        }
        case 8:
        case 9:
        {
            videoRam[addr - 0x8000] = val;
            return;
        }
        case 10:
        case 11:
        {
            access_cartridge->writeToRam(addr - 0xa000, val);
            return;
        }
        case 12:
        case 13:
        {
            workRam[addr - 0xc000] = val;
            return;
        }
        case 14:
            return;
        case 15:
        {
            if (addr < 0xfe00)
                return;
            else if (addr < 0xfea0)
                oam[addr - 0xfe00] = val;
            else if (addr < 0xff00)
                return;
            else 
            {
                byte& value_to_change = last0x100[addr - 0xff00];
                switch (addr) 
                {
                    case Memory::DIVIDER_REGISTER: { value_to_change = 0; break; }
                    case Memory::OAM_DMA_ADDR:  
                    { 
                        oamDma(val);
                        value_to_change = val; 
                        break; 
                    }
                    case Memory::TIMER_CONTROL: 
                    {
                        unsigned short duration = Scheduler::TIMA_PERIODS[val & 0b0011];
                        pScheduler->replace(UPDATE_TIMA, duration); 
                        value_to_change = val;
                        break;
                    }
                    case Memory::LCD_STAT:
                    {
                        value_to_change = (val & 0b11111100) + (value_to_change & 0b11); 
                        pScheduler->statInterruptCheck();
                        break;
                    }
                    case Memory::NR10: 
                    {
                        if (pApu->isOn())
                        {
                            u8 pace = val & 0b01110000;
                            if (!pace)
                                pScheduler->remove(CH1_SWEEP);
                            else if (!(value_to_change & 0b01110000))
                                pScheduler->push(pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::CH1_SWEEP_FREQUENCY, CH1_SWEEP);
                            value_to_change = val;
                        }
                        break;
                    }
                    case Memory::NR11: 
                    { 
                        value_to_change = val;
                        if (pApu->isOn())
                            pApu->nr11Changed();
                        break; 
                    }
                    case Memory::NR12: 
                    { 
                        if (pApu->isOn())
                        {
                            u8 pace = val & 0b0111;
                            if (!pace) 
                                pScheduler->remove(CH1_ENVELOPE);
                            else if (!(value_to_change & 0b0111))
                                pScheduler->push(pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH1_ENVELOPE);

                            value_to_change = val;
                            pApu->nr12Changed();
                        }
                        break; 
                    }
                    case Memory::NR14: 
                    { 
                        if (pApu->isOn())
                        {
                            if (val & 0b10000000)
                            {
                                pScheduler->remove(CH1_SWEEP);
                                pScheduler->remove(CH1_ENVELOPE);
                                pScheduler->remove(CH1_TIME);

                                u8 sweep_pace = (last0x100[Memory::NR10 - 0xff00] >> 4) & 0b0111;
                                if (sweep_pace) 
                                    pScheduler->push(sweep_pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::CH1_SWEEP_FREQUENCY, CH1_SWEEP);

                                u8 envelope_pace = last0x100[Memory::NR12 - 0xff00] & 0b0111;
                                if (envelope_pace) 
                                    pScheduler->push(envelope_pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH1_ENVELOPE);

                                if (val & 0b01000000) 
                                    pScheduler->push(Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::SOUND_TIMER_FREQUENCY, CH1_TIME);
                            }
                            value_to_change = val;
                            pApu->nr14Changed();
                        }
                        break;
                    }
                    case Memory::NR21: 
                    { 
                        value_to_change = val;
                        if (pApu->isOn())
                            pApu->nr21Changed(); 
                        break; 
                    }
                    case Memory::NR22: 
                    { 
                        if (pApu->isOn())
                        {
                            u8 pace = val & 0b0111;
                            if (!pace) 
                                pScheduler->remove(CH2_ENVELOPE);
                            else if (!(value_to_change & 0b0111))
                                pScheduler->push(pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH2_ENVELOPE);
 
                            value_to_change = val;
                            pApu->nr22Changed();
                        }
                        break; 
                    }
                    case Memory::NR24: 
                    { 
                        if (pApu->isOn())
                        {
                            if (val & 0b10000000)
                            {
                                pScheduler->remove(CH2_ENVELOPE);
                                pScheduler->remove(CH2_TIME);

                                u8 envelope_pace = last0x100[Memory::NR22 - 0xff00] & 0b0111;
                                if (envelope_pace) 
                                    pScheduler->push(envelope_pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH2_ENVELOPE);

                                if (val & 0b01000000) 
                                    pScheduler->push(Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::SOUND_TIMER_FREQUENCY, CH2_TIME);
                            }
    
                            value_to_change = val;
                            pApu->nr24Changed();
                        }
                        break;
                    }
                    case Memory::NR30:
                    {
                        value_to_change = val;
                        if (pApu->isOn())
                            pApu->nr30Changed();
                        break;
                    } 
                    case Memory::NR32:
                    {
                        value_to_change = val;
                        if (pApu->isOn())
                            pApu->nr32Changed();
                        break;
                    }
                    case Memory::NR34:
                    {
                        if (pApu->isOn())
                        {
                            if (val & 0b10000000)
                            {
                                pScheduler->remove(CH3_TIME);

                                if (val & 0b01000000) 
                                    pScheduler->push(Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::SOUND_TIMER_FREQUENCY, CH3_TIME);
                            }

                            value_to_change = val;
                            pApu->nr34Changed();
                        }
                        break;
                    }
                    case Memory::NR42:
                    {
                        if (pApu->isOn())
                        {
                            u8 pace = val & 0b0111;
                            if (!pace) 
                                pScheduler->remove(CH4_ENVELOPE);
                            else if (!(value_to_change & 0b0111))
                                pScheduler->push(pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH4_ENVELOPE);

                            value_to_change = val;
                            pApu->nr42Changed();
                        }
                        break; 
                    }
                    case Memory::NR43:
                    {
                        value_to_change = val;
                        if (pApu->isOn())
                            pApu->nr43Changed();
                        break;
                    }
                    case Memory::NR44:
                    {
                        if (pApu->isOn())
                        {
                            if (val & 0b10000000)
                            {
                                pScheduler->remove(CH4_SHIFT);
                                pScheduler->remove(CH4_ENVELOPE);
                                pScheduler->remove(CH4_TIME);

                                byte nr43 = last0x100[Memory::NR43 - 0xff00];

                                float divider = nr43 & 0b0111;
                                if (!divider)
                                    divider = 0.5;

                                float shift = 1 << (nr43 >> 4);

                                pScheduler->push(Scheduler::CH4_SHIFT_TIME / (divider * shift), CH4_SHIFT);

                                u8 envelope_pace = last0x100[Memory::NR42 - 0xff00] & 0b0111;
                                if (envelope_pace) 
                                    pScheduler->push(envelope_pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH4_ENVELOPE);

                                if (val & 0b01000000) 
                                    pScheduler->push(Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::SOUND_TIMER_FREQUENCY, CH4_TIME);

                            }
 
                            value_to_change = val;
                            pApu->nr44Changed();
                        }
                        break;
                    }
                    case Memory::NR51: 
                    {
                        value_to_change = val;
                        if (pApu->isOn())
                            pApu->soundPanningChanged(); 
                        break; 
                    }
                    case Memory::NR52: 
                    { 
                        bool on_before = value_to_change & 0b10000000;
                        value_to_change = (val & 0b11110000) + (value_to_change & 0b00001111);
                        bool on_now = value_to_change & 0b10000000;
                        if (!on_before && on_now)
                            pScheduler->push(0, SAMPLE);
                        else if (on_before && !on_now)
                            pScheduler->remove(SAMPLE);
                        pApu->audioMasterChanged(); 
                        break; 
                    }
                    default:{ value_to_change = val; break; }
                }
            }
        }
        default:
            return;
    }
}

word Memory::operator()(unsigned short ind) { return (read(ind + 1) << 8) + read(ind); }

void Memory::writeWord(unsigned short addr, word val)
{
    write(addr, val >> 8);       
    write(addr, val);
}

byte* Memory::getDataPointerToAddress(unsigned short addr)
{
    auto access_cartridge = pCartridge.lock();
    //if (!access_cartridge)
    //{
    //    std::cout << "Cartridge pointer expired" << std::endl;
    //    return NULL;
    //}

    switch (addr >> 12)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        {
            if (!access_cartridge)
                return cartridgePtrError();
            return access_cartridge->getBankPointer(addr);
        }
        case 4:
        case 5:
        case 6:
        case 7:
        {
            if (!access_cartridge)
                return cartridgePtrError();
            return access_cartridge->getBankNPointer(addr - 0x4000);
        }
        case 8:
        case 9:
            return videoRam + addr - 0x8000;
        case 10:
        case 11:
        {
            if (!access_cartridge)
                return cartridgePtrError();
            return access_cartridge->getRamPointer(addr - 0xa000);
        }
        case 12:
        case 13:
            return workRam + addr - 0xc000;
        case 14:
            return NULL;
        case 15:
        {
            if (addr < 0xfe00)
                return NULL;
            else if (addr < 0xfea0)
                return oam + addr - 0xfe00;
            else if (addr < 0xff00)
                return NULL;
            else 
                return last0x100 + addr - 0xff00;
        }
        default:
            return NULL;
    }
}

void Memory::oamDma(byte val) 
{ 
    if (val < 0xe0)
        memcpy(oam, getDataPointerToAddress(val << 8), 0xa0); 
}

byte& Memory::buildIn(unsigned short addr) { return *getDataPointerToAddress(addr); }

byte* Memory::cartridgePtrError() 
{
    std::cout << "Cartridge pointer expired" << std::endl;
    return NULL;
}

