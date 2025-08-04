cmake_minimum_required(VERSION 4.0.0)

add_library(controller controller.cc)

target_include_directories(controller PRIVATE ../general)
target_include_directories(controller PRIVATE ../mem)

