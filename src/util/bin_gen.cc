#include <cstdio>
#include <iomanip>
#include <iostream>

int main() {
    FILE* file = fopen("./src/gameboy/dmg_boot.bin", "rb");
    unsigned char c[1];
    int count = 0;
    std::cout << "{\n    ";
    while (fread(c, 1, 1, file)) 
    {
        std::cout << std::setfill(' ') << std::setw(3) << (int) c[0] << ", ";
        if (!(++count %= 16)) 
        {
            std::cout << "\n    ";
        }
    }
    std::cout << "}";
}
