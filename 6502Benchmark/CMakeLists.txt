cmake_minimum_required(VERSION 3.14)
project( 6502Benchmark )
enable_testing()

include(FetchContent)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG master) # need master for benchmark::benchmark

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googlebenchmark)

# source for the test executable
set  (6502_Benchmark_SOURCES
        "main.cpp"
        "_6502LoadRegisterBenchmarks.cpp")

add_executable( 6502Benchmark ${6502_Benchmark_SOURCES} 	)
add_dependencies( 6502Benchmark 6502Lib )
target_link_libraries(6502Benchmark benchmark::benchmark 6502Lib)