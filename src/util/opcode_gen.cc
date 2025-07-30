#include <iomanip>
#include <iostream>
#include <ostream>

int main() {
    for(int i = 0; i < 0x100; i++) {
        std::cout << "inline void op_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "(); " ;
        if (!((i + 1) % 8))
            std::cout << std::endl;
    }
    std::cout << std::endl;
    
    for(int i = 0; i < 0x100; i++) {
        std::cout << "inline void op_cb_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "(); " ;
        if (!((i + 1) % 8))
            std::cout << std::endl;
    }
    std::cout << std::endl;

    //for(int i = 0; i < 0x100; i++) {
    //    std::cout << "inline void Cpu::op_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "(); " ;
    //    if (!((i + 1) % 8))
    //        std::cout << std::endl;
    //}

    //std::cout << std::endl;
    //for(int i = 0; i < 0x100; i++) {
    //    std::cout << "inline void op_cb_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "(); " ;
    //    if (!((i + 1) % 8))
    //        std::cout << std::endl;
    //}

}
