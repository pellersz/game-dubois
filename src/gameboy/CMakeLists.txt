cmake_minimum_required(VERSION 4.0.0)

add_library(gameboy gameboy.cc)

target_include_directories(gameboy PRIVATE  ../cpu)
target_include_directories(gameboy PRIVATE ../general)
target_include_directories(gameboy PRIVATE ../mem)
target_include_directories(gameboy PRIVATE ../scheduler)
target_include_directories(gameboy PRIVATE ../controller)
target_include_directories(gameboy PRIVATE ../cartridge)

target_link_libraries(gameboy PRIVATE controller)
