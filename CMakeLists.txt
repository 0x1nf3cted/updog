

cmake_minimum_required(VERSION 3.5)

project(updog)
include_directories(include)

add_executable(updog src/server.c src/utils.c  src/client.c src/main.c)

target_include_directories(updog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(updog PUBLIC pthread)
