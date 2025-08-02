cmake_minimum_required(VERSION 4.0.0)

add_library(scheduler scheduler.cc)

target_include_directories(scheduler PRIVATE ../general)
target_include_directories(scheduler PRIVATE ../cpu)
target_include_directories(scheduler PRIVATE ../mem)

