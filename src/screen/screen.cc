#include "screen.h"
#include "GLFW/glfw3.h"
#include "types.h"
#include <GL/gl.h>
#include <cstring>
#include <iostream>

Screen::Screen() 
{
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

GLFWwindow* Screen::getWindow() { return window; } 

byte& Screen::operator()(u8 height, u8 width) 
{
    if (height < LCD_HEIGHT && width < LCD_WIDTH)
        return lcd[(LCD_HEIGHT - height - 1) * LCD_WIDTH + width];
    return dummy;
}

bool Screen::shouldClose() { return glfwWindowShouldClose(window); }

void Screen::fillWhite(u8 y) { memset(lcd + y * LCD_WIDTH, 255, LCD_WIDTH); }

void Screen::updateFrame() 
{ 
    // This might be deprecated, but for a gameboy this should be fine.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPixelZoom(5, 5);
    glDrawPixels
    (
        LCD_WIDTH,
        LCD_HEIGHT,
        GL_LUMINANCE,
        GL_UNSIGNED_BYTE,
        lcd
    );
    glfwSwapBuffers(window);
}

