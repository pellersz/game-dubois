cmake_minimum_required(VERSION 4.0.0)

add_library(scheduler scheduler.cc)

target_include_directories(scheduler PRIVATE ../general)
target_include_directories(scheduler PRIVATE ../cpu)
target_include_directories(scheduler PRIVATE ../mem)
target_include_directories(scheduler PRIVATE ../controller)
target_include_directories(scheduler PRIVATE ../screen)
target_include_directories(scheduler PRIVATE ../apu)

target_link_libraries(scheduler glfw)
target_link_libraries(scheduler miniaudio)
