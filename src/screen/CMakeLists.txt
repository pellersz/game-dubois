cmake_minimum_required(VERSION 4.0.0)

add_library(screen screen.cc ppu.cc)

target_include_directories(screen PRIVATE ../general)
target_include_directories(screen PRIVATE ../../external/glfw)
target_include_directories(screen PRIVATE ../mem)
target_link_directories(screen PRIVATE external/glfw/src)
target_link_libraries(screen glfw -lGL -lGLU -lX11)


