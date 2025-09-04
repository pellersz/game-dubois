#include "controller.h"
#include "mem.h"
#include "types.h"

Controller::Controller(Memory& memory) : memory(memory) {}
   
void Controller::buttonPressed(byte button) { pressedVector &= ~button; }

void Controller::buttonReleased(byte button) { pressedVector |= button; }

void Controller::updatePressed() 
{
    byte joypad = memory.read(Memory::JOYPAD);
    byte tmp = joypad;

    byte new_val;
    if (~joypad & 0b00100000) 
        new_val = (joypad & 0b11110000) + (pressedVector & 0b00001111);
    else
        new_val = (joypad & 0b11110000) + (pressedVector >> 4);

    memory.write(Memory::JOYPAD, new_val);

    if (new_val < tmp)
        memory.write(Memory::INTERRUPT_FLAG, memory.read(Memory::INTERRUPT_FLAG) | 0b00010000);       
}

