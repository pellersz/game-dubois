cmake_minimum_required(VERSION 4.0.0)

add_library(apu apu.cc speaker.cc)

target_include_directories(apu PRIVATE ../general)

