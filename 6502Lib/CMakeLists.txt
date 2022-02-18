cmake_minimum_required(VERSION 3.14)

project( 6502Lib )

set  (6502_LIB_SOURCES
        "6502.h"
        "6502.cpp"
        "main.cpp")

add_library( 6502Lib ${6502_LIB_SOURCES} )

add_executable(main ${6502_LIB_SOURCES})

target_include_directories ( 6502Lib PUBLIC "${PROJECT_SOURCE_DIR}")