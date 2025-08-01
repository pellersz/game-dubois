cmake_minimum_required(VERSION 4.0.0)

add_library(mem mem.cc)

target_include_directories(mem PRIVATE ../general)

