cmake_minimum_required(VERSION 4.0.0)
project(game_boi)

add_executable(${PROJECT_NAME} src/main.cc)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

add_subdirectory(src/gameboy)
target_include_directories(${PROJECT_NAME} PUBLIC src/gameboy)
target_link_directories(${PROJECT_NAME} PRIVATE src/gameboy)
target_link_libraries(${PROJECT_NAME} gameboy)

add_subdirectory(src/general)
target_include_directories(${PROJECT_NAME} PUBLIC src/general)
target_link_directories(${PROJECT_NAME} PRIVATE src/general)

add_subdirectory(src/cpu)
target_include_directories(${PROJECT_NAME} PUBLIC src/cpu)
target_link_directories(${PROJECT_NAME} PRIVATE src/cpu)
target_link_libraries(${PROJECT_NAME} cpu)

add_subdirectory(src/mem)
target_include_directories(${PROJECT_NAME} PUBLIC src/mem)
target_link_directories(${PROJECT_NAME} PRIVATE src/mem)
target_link_libraries(${PROJECT_NAME} mem)

add_subdirectory(src/scheduler)
target_include_directories(${PROJECT_NAME} PUBLIC src/scheduler)
target_link_directories(${PROJECT_NAME} PRIVATE src/scheduler)
target_link_libraries(${PROJECT_NAME} scheduler)

add_subdirectory(src/controller)
target_include_directories(${PROJECT_NAME} PUBLIC src/controller)
target_link_directories(${PROJECT_NAME} PRIVATE src/controller)
target_link_libraries(${PROJECT_NAME} controller)

add_subdirectory(src/cartridge)
target_include_directories(${PROJECT_NAME} PUBLIC src/cartridge)
target_link_directories(${PROJECT_NAME} PRIVATE src/cartridge)
target_link_libraries(${PROJECT_NAME} cartridge)

add_subdirectory(src/screen)
target_include_directories(${PROJECT_NAME} PUBLIC src/screen)
target_link_directories(${PROJECT_NAME} PRIVATE src/screen)
target_link_libraries(${PROJECT_NAME} screen)

add_subdirectory(external/glfw)
target_include_directories(${PROJECT_NAME} PUBLIC external/glfw)
target_link_directories(${PROJECT_NAME} PRIVATE external/glfw/src)
target_link_libraries(${PROJECT_NAME} glfw)

