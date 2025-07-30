cmake_minimum_required(VERSION 4.0.0)
project(game_boi)

add_executable(${PROJECT_NAME} src/main.cc)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

