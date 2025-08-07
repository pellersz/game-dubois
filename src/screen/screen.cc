#include "screen.h"
#include "GLFW/glfw3.h"
#include "types.h"
#include <cstring>
#include <iostream>

Screen::Screen() {
    if (!glfwInit()) 
    {
        std::cout << "Could not initialize glfw";
        throw 1;
    }
    
    if (!(window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "GameBoi", NULL, NULL)))
    {
        std::cout << "Could not create window";
        throw 2;
    }  

    glfwMakeContextCurrent(window);
}

byte& Screen::operator()(u8 height, u8 width) {
    if (height < LCD_HEIGHT && width < LCD_WIDTH)
        return lcd[height * WIN_WIDTH + width];
    return dummy;
}

bool Screen::shouldClose() { return glfwWindowShouldClose(window); }

void Screen::fillWhite(u8 y) { memset(lcd + y * LCD_WIDTH, 255, LCD_WIDTH); }

