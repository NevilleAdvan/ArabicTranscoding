cmake_minimum_required(VERSION 3.12)
project(FribidiTest)

set(CMAKE_CXX_STANDARD 20)

add_executable(FribidiTest main.cpp)

target_link_libraries(FribidiTest fribidi)
