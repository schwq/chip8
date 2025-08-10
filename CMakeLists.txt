cmake_minimum_required(VERSION 3.0)
project(chip8)

set(sourceFiles 
    source/main.cc 
    source/chip8.cc
)

add_executable(chip8 ${sourceFiles})
include_directories(deps/SDL2)
target_link_libraries(chip8 m SDL2main SDL2)
add_definitions(-w -g)