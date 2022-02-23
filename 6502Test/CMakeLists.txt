cmake_minimum_required(VERSION 3.14)
project( 6502Test )
enable_testing()

include(FetchContent)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.10.x)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# source for the test executable
set  (6502_TEST_SOURCES
        "_6502LoadRegisterTests.cpp"
        "_6502StoreRegisterTests.cpp"
        "_6502JumpsAndCallsTests.cpp"
        "_6502StackOperationTests.cpp"
        "main.cpp")

add_executable( 6502Test ${6502_TEST_SOURCES} 	)
add_dependencies( 6502Test 6502Lib )
target_link_libraries(6502Test gtest 6502Lib)