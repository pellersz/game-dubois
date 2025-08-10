#include <iomanip>
#include <iostream>
#include <ostream>

int main() {
    //for(int i = 0; i < 0x100; i++) {
    //    std::cout << "void op_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "(); " ;
    //    if (!((i + 1) % 8))
    //        std::cout << std::endl;
    //}
    //std::cout << std::endl;
    
    //for(int i = 0; i < 0x100; i++) {
    //    std::cout << "void op_cb_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "(); " ;
    //    if (!((i + 1) % 8))
    //        std::cout << std::endl;
    //}
    //std::cout << std::endl;

    //for(int i = 0; i < 0x100; i++) {
    //    std::cout << "void Cpu::op_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "(); " ;
    //    if (!((i + 1) % 8))
    //        std::cout << std::endl;
    //}

    //std::cout << std::endl;
    //for(int i = 0; i < 0x100; i++) {
    //    std::cout << "void op_cb_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "(); " ;
    //    if (!((i + 1) % 8))
    //        std::cout << std::endl;
    //}

    //for(int i = 0; i < 0x100; i++) {
    //    std::cout << "case 0x" << std::hex << std::setfill('0') << std::setw(2) << i << ": {op_0x" << std::setfill('0') << std::setw(2) << i << "(); break;} " ;
    //    if (!((i + 1) % 4))
    //        std::cout << std::endl;
    //}
    //std::cout << std::endl;

    //for(int i = 0; i < 0x100; i++) {
    //    std::cout << "case 0x" << std::hex << std::setfill('0') << std::setw(2) << i << ": {opCb_0x" << std::setfill('0') << std::setw(2) << i << "(); break;} " ;
    //    if (!((i + 1) % 4))
    //        std::cout << std::endl;
    //}
    //std::cout << std::endl;
 
    for(int i = 0; i < 0x100; i++) {
        std::cout << "void Cpu::op_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "() {} " ;
        std::cout << std::endl << std::endl;
    }
    std::cout << std::endl;
    
    for(int i = 0; i < 0x100; i++) {
        std::cout << "void Cpu::opCb_0x" << std::hex << std::setfill('0') << std::setw(2) << i << "() {} " ;
        std::cout << std::endl << std::endl;
    }
    std::cout << std::endl;

}
