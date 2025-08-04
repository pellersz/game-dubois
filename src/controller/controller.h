#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "mem.h"
#include "types.h"

class Controller {
public:
    static const byte A_PRESSED      = 0b00000001;
    static const byte B_PRESSED      = 0b00000010;
    static const byte SELECT_PRESSED = 0b00000100;
    static const byte START_PRESSED  = 0b00001000;
    static const byte RIGHT_PRESSED  = 0b00010000;
    static const byte LEFT_PRESSED   = 0b00100000;
    static const byte UP_PRESSED     = 0b01000000;
    static const byte DOWN_PRESSED   = 0b10000000;

    Controller(Memory&);
    void buttonPressed(byte);
    void buttonReleased(byte);
    void updatePressed();
    Controller& operator=(Controller&);

private:
    // vector for identifying which buttons are pressed
    volatile byte pressedVector = 0;
    Memory& memory;
};

#endif
