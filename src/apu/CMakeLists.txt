cmake_minimum_required(VERSION 4.0.0)

add_library(apu Apu.cpp Speaker.cpp)

target_include_directories(apu PRIVATE ../general)
target_include_directories(apu PRIVATE ../../external/miniaudio)
target_include_directories(apu PRIVATE ../mem)

target_link_directories(apu PRIVATE external/miniaudio)
target_link_libraries(apu miniaudio)


