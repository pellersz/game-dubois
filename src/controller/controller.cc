#include "controller.h"
#include "mem.h"
#include "types.h"

Controller::Controller(Memory& memory) : memory(memory) {}
   
void Controller::buttonPressed(byte button) { pressedVector |= button; }

void Controller::buttonReleased(byte button) { pressedVector &= ~button; }

void Controller::updatePressed() 
{
    byte& joypad = memory[Memory::JOYPAD];
    if (joypad & 0b00100000) 
    {
        joypad = (joypad & 0b11110000) + (pressedVector & 0b00001111);
        return;
    }
    joypad = (joypad & 0b11110000) + (pressedVector >> 4);
}

Controller& Controller::operator=(Controller& other) 
{
    memory = other.memory;
    return *this;
}
