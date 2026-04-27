#include "Memory.hpp"
#include "Controller.hpp"
#include "Scheduler.hpp"
#include "Apu.hpp"
#include "Cartridge.hpp"
#include "types.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <unistd.h>

void Memory::init
(
    Scheduler *pScheduler,
    Apu *pApu,
    Controller *pController
) 
{
    this->pScheduler = pScheduler;
    this->pApu = pApu;
    this->pController = pController;
}

void Memory::load(std::shared_ptr<Cartridge> pCartridge) { this->pCartridge = pCartridge; }

byte Memory::read(unsigned short addr)
{
    auto accessCartridge = pCartridge.lock();
    if (!accessCartridge)
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
            return accessCartridge->readBank(addr);
        case 4:
        case 5:
        case 6:
        case 7:
            return accessCartridge->readBankN(addr - 0x4000);
        case 8:
        case 9:
            return videoRam[addr - 0x8000];
        case 10:
        case 11:
            return accessCartridge->readRam(addr - 0xa000);
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
    auto accessCartridge = pCartridge.lock();
    if (!accessCartridge)
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
            accessCartridge->writeToRegister(addr, val);
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
            accessCartridge->writeToRam(addr - 0xa000, val);
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
                byte& valueToChange = last0x100[addr - 0xff00];
                switch (addr) 
                {
                    case Memory::JOYPAD: 
                    { 
                        valueToChange = val;
                        pController->updatePressed();
                        break; 
                    }
                    case Memory::DIVIDER_REGISTER: { valueToChange = 0; break; }
                    case Memory::OAM_DMA_ADDR:  
                    { 
                        oamDma(val);
                        valueToChange = val; 
                        break; 
                    }
                    case Memory::TIMER_CONTROL: 
                    {
                        unsigned short duration = Scheduler::TIMA_PERIODS[val & 0b0011];
                        pScheduler->replace(UPDATE_TIMA, duration); 
                        valueToChange = val;
                        break;
                    }
                    case Memory::LCD_STAT:
                    {
                        valueToChange = (val & 0b11111100) + (valueToChange & 0b11); 
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
                            else if (!(valueToChange & 0b01110000))
                                pScheduler->push(pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::CH1_SWEEP_FREQUENCY, CH1_SWEEP);
                            valueToChange = val;
                        }
                        break;
                    }
                    case Memory::NR11: 
                    { 
                        valueToChange = val;
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
                            else if (!(valueToChange & 0b0111))
                                pScheduler->push(pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH1_ENVELOPE);

                            valueToChange = val;
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

                                u8 sweepPace = (last0x100[Memory::NR10 - 0xff00] >> 4) & 0b0111;
                                if (sweepPace) 
                                    pScheduler->push(sweepPace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::CH1_SWEEP_FREQUENCY, CH1_SWEEP);

                                u8 envelopePace = last0x100[Memory::NR12 - 0xff00] & 0b0111;
                                if (envelopePace) 
                                    pScheduler->push(envelopePace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH1_ENVELOPE);

                                if (val & 0b01000000) 
                                    pScheduler->push(Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::SOUND_TIMER_FREQUENCY, CH1_TIME);
                            }
                            valueToChange = val;
                            pApu->nr14Changed();
                        }
                        break;
                    }
                    case Memory::NR21: 
                    { 
                        valueToChange = val;
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
                            else if (!(valueToChange & 0b0111))
                                pScheduler->push(pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH2_ENVELOPE);
 
                            valueToChange = val;
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

                                u8 envelopePace = last0x100[Memory::NR22 - 0xff00] & 0b0111;
                                if (envelopePace) 
                                    pScheduler->push(envelopePace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH2_ENVELOPE);

                                if (val & 0b01000000) 
                                    pScheduler->push(Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::SOUND_TIMER_FREQUENCY, CH2_TIME);
                            }
    
                            valueToChange = val;
                            pApu->nr24Changed();
                        }
                        break;
                    }
                    case Memory::NR30:
                    {
                        valueToChange = val;
                        if (pApu->isOn())
                            pApu->nr30Changed();
                        break;
                    } 
                    case Memory::NR32:
                    {
                        valueToChange = val;
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

                            valueToChange = val;
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
                            else if (!(valueToChange & 0b0111))
                                pScheduler->push(pace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH4_ENVELOPE);

                            valueToChange = val;
                            pApu->nr42Changed();
                        }
                        break; 
                    }
                    case Memory::NR43:
                    {
                        valueToChange = val;
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

                                u8 envelopePace = last0x100[Memory::NR42 - 0xff00] & 0b0111;
                                if (envelopePace) 
                                    pScheduler->push(envelopePace * Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::ENVELOPE_FREQUENCY, CH4_ENVELOPE);

                                if (val & 0b01000000) 
                                    pScheduler->push(Scheduler::MASTER_CLOCK_FREQUENCY / Scheduler::SOUND_TIMER_FREQUENCY, CH4_TIME);

                            }
 
                            valueToChange = val;
                            pApu->nr44Changed();
                        }
                        break;
                    }
                    case Memory::NR51: 
                    {
                        valueToChange = val;
                        if (pApu->isOn())
                            pApu->soundPanningChanged(); 
                        break; 
                    }
                    case Memory::NR52: 
                    { 
                        bool onBefore = valueToChange & 0b10000000;
                        valueToChange = (val & 0b11110000) + (valueToChange & 0b00001111);
                        bool onNow = valueToChange & 0b10000000;
                        if (!onBefore && onNow)
                            pScheduler->push(0, SAMPLE);
                        else if (onBefore && !onNow)
                            pScheduler->remove(SAMPLE);
                        pApu->audioMasterChanged(); 
                        break; 
                    }
                    default:{ valueToChange = val; break; }
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
    auto accessCartridge = pCartridge.lock();

    switch (addr >> 12)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        {
            if (!accessCartridge)
                return cartridgePtrError();
            return accessCartridge->getBankPointer(addr);
        }
        case 4:
        case 5:
        case 6:
        case 7:
        {
            if (!accessCartridge)
                return cartridgePtrError();
            return accessCartridge->getBankNPointer(addr - 0x4000);
        }
        case 8:
        case 9:
            return videoRam + addr - 0x8000;
        case 10:
        case 11:
        {
            if (!accessCartridge)
                return cartridgePtrError();
            return accessCartridge->getRamPointer(addr - 0xa000);
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

