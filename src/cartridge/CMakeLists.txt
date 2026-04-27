cmake_minimum_required(VERSION 4.0.0)

add_library(cartridge Cartridge.cpp Mbc1.cpp)

target_include_directories(
    cartridge 
    PRIVATE ../general
    PRIVATE ../mem
)

target_link_libraries(
    cartridge
    mem
)
