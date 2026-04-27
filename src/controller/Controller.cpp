#include "Controller.hpp"
#include "Memory.hpp"
#include "types.h"

Controller::Controller(Memory& memory) : 
    memory(memory), 
    joypad(memory.buildIn(Memory::JOYPAD)),
    interruptFlag(memory.buildIn(Memory::INTERRUPT_FLAG))
{}
   
void Controller::buttonPressed(byte button) { pressedVector &= ~button; }

void Controller::buttonReleased(byte button) { pressedVector |= button; }

void Controller::handleInput(GLFWwindow* window) 
{
    static const std::pair<int, byte> BUTTONS[] = 
    {
        {GLFW_KEY_RIGHT,      Controller::RIGHT },
        {GLFW_KEY_LEFT,       Controller::LEFT  },
        {GLFW_KEY_DOWN,       Controller::DOWN  },
        {GLFW_KEY_UP,         Controller::UP    },
        {GLFW_KEY_Z,          Controller::A     },
        {GLFW_KEY_X,          Controller::B     },
        {GLFW_KEY_ENTER,      Controller::START },
        {GLFW_KEY_LEFT_SHIFT, Controller::SELECT},
    };

    glfwPollEvents();
    for (int i = 0; i < 8; ++i) {
        if(glfwGetKey(window, BUTTONS[i].first) == GLFW_PRESS)
            buttonPressed(BUTTONS[i].second);
        else 
            buttonReleased(BUTTONS[i].second);
    }

    updatePressed();
}

void Controller::updatePressed() 
{
    byte tmp = joypad;

    byte newVal;
    if (~joypad & 0b00100000) 
        newVal = (joypad & 0b11110000) + (pressedVector & 0b00001111);
    else
        newVal = (joypad & 0b11110000) + (pressedVector >> 4);

    joypad = newVal;

    if (newVal < tmp)
        interruptFlag = interruptFlag | 0b00010000;       
}

