#include "controller.h"
#include "mem.h"
#include "types.h"

Controller::Controller(Memory& memory) : 
    memory(memory), 
    joypad(memory.buildIn(Memory::JOYPAD)),
    interruptFlag(memory.buildIn(Memory::INTERRUPT_FLAG))
{}
   
void Controller::buttonPressed(byte button) { pressedVector &= ~button; }

void Controller::buttonReleased(byte button) { pressedVector |= button; }

void Controller::updatePressed() 
{
    byte tmp = joypad;

    byte new_val;
    if (~joypad & 0b00100000) 
        new_val = (joypad & 0b11110000) + (pressedVector & 0b00001111);
    else
        new_val = (joypad & 0b11110000) + (pressedVector >> 4);

    joypad = new_val;

    if (new_val < tmp)
        interruptFlag = interruptFlag | 0b00010000;       
}

