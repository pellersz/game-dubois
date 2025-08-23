cmake_minimum_required(VERSION 4.0.0)
project(game_boi)

add_executable(${PROJECT_NAME} src/main.cc)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

add_subdirectory(src/gameboy)
add_subdirectory(src/general)
add_subdirectory(src/cpu)
add_subdirectory(src/mem)
add_subdirectory(src/scheduler)
add_subdirectory(src/controller)
add_subdirectory(src/cartridge)
add_subdirectory(src/screen)
add_subdirectory(src/apu)
add_subdirectory(external/glfw)
add_subdirectory(external/miniaudio)

target_include_directories(
    ${PROJECT_NAME} 
    PUBLIC src/gameboy
    PUBLIC src/general
    PUBLIC src/cpu
    PUBLIC src/mem
    PUBLIC src/scheduler
    PUBLIC src/controller
    PUBLIC src/cartridge
    PUBLIC src/screen
    PUBLIC src/apu
    PUBLIC external/glfw
    PUBLIC external/miniaudio
)

target_link_directories(
    ${PROJECT_NAME} 
    PRIVATE src/gameboy
    PRIVATE src/general
    PRIVATE src/cpu
    PRIVATE src/mem
    PRIVATE src/scheduler
    PRIVATE src/cartridge
    PRIVATE src/screen
    PRIVATE src/apu
    PRIVATE external/glfw/src
)

target_link_libraries(
    ${PROJECT_NAME} 
    gameboy
    cpu
    mem
    scheduler
    controller
    cartridge
    screen
    apu
    glfw
    miniaudio
)

