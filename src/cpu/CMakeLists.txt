cmake_minimum_required(VERSION 4.0.0)

add_library(cpu cpu.cc ops_impl.cc)

target_include_directories(cpu PRIVATE ../general)
target_include_directories(cpu PRIVATE ../mem)
target_include_directories(cpu PRIVATE ../scheduler)
target_include_directories(cpu PRIVATE ../controller)
target_include_directories(cpu PRIVATE ../screen)
target_include_directories(cpu PRIVATE ../apu)

target_link_libraries(cpu PRIVATE glfw)
target_link_libraries(cpu PRIVATE miniaudio)
# this might sound dumb (and I am sure that I did something stupid, but the mutual include needs this)
target_include_directories(cpu PRIVATE .)
