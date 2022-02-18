cmake_minimum_required(VERSION 3.14)

project( 6502_emulation )

## Project-wide setup
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_subdirectory(6502Test)
add_subdirectory(6502Lib)
add_subdirectory(6502Benchmark)