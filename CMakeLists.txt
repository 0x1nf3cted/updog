cmake_minimum_required(VERSION 3.5)

project(updog)
include_directories(include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

FILE(GLOB_RECURSE sources src/*.c)
add_executable(updog ${sources})

target_include_directories(updog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(updog PUBLIC pthread ${CURSES_LIBRARY})
