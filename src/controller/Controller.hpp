#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "GLFW/glfw3.h"
#include "types.h"

class Memory;

class Controller 
{
  public:
    static const byte A      = 0b00000001;
    static const byte B      = 0b00000010;
    static const byte SELECT = 0b00000100;
    static const byte START  = 0b00001000;
    static const byte RIGHT  = 0b00010000;
    static const byte LEFT   = 0b00100000;
    static const byte UP     = 0b01000000;
    static const byte DOWN   = 0b10000000;

    Controller(Memory&);
    
    void buttonPressed(byte);
    void buttonReleased(byte);
    void handleInput(GLFWwindow*);
    void updatePressed();

  private:
    // vector for identifying which buttons are pressed
    volatile byte pressedVector = 0;
    
    Memory& memory;

    byte& joypad;
    byte& interruptFlag;
};

#endif
