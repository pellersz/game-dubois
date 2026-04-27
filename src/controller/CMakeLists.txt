cmake_minimum_required(VERSION 4.0.0)

add_library(controller Controller.cpp)

target_include_directories(controller PRIVATE ../general)
target_include_directories(controller PRIVATE ../mem)
