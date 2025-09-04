cmake_minimum_required(VERSION 4.0.0)

add_library(mem mem.cc)

target_include_directories(
    mem 
    PRIVATE ../general
    PRIVATE ../cartridge
    PRIVATE ../scheduler
    PRIVATE ../controller
    PRIVATE ../cpu
    PRIVATE ../apu
    PRIVATE ../../external/miniaudio
)

