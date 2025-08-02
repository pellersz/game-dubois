cmake_minimum_required(VERSION 4.0.0)

add_library(cpu cpu.cc ops_impl.cc)

target_include_directories(cpu PRIVATE ../general)
target_include_directories(cpu PRIVATE ../mem)
target_include_directories(cpu PRIVATE ../scheduler)

