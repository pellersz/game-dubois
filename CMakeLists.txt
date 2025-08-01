cmake_minimum_required(VERSION 4.0.0)
project(game_boi)

add_executable(${PROJECT_NAME} src/main.cc)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

add_subdirectory(src/cpu)
target_include_directories(${PROJECT_NAME} PUBLIC src/cpu)
target_link_directories(${PROJECT_NAME} PRIVATE src/cpu)
target_link_libraries(${PROJECT_NAME} cpu)

