#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"
#include "GLFW/glfw3.h"

class Screen {
public:
    static const u8 LCD_HEIGHT = 144;
    static const u8 LCD_WIDTH = 160;

    Screen();
    GLFWwindow* getWindow();
    byte& operator()(u8, u8);
    bool shouldClose();
    void fillWhite(u8);
    void updateFrame();

private:
    static const u8 WIN_HEIGHT = 150;
    static const u8 WIN_WIDTH = 170;
    GLFWwindow* window;

    byte dummy;
    byte lcd[LCD_HEIGHT * LCD_WIDTH];
};

#endif
