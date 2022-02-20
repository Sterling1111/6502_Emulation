#include "gtest/gtest.h"
#include "6502.h"

using namespace m6502;

class _6502JumpsAndCallsTests : public testing::Test {
public:
    CPU cpu;
    virtual void SetUp() { cpu.reset(); }
    virtual void TearDown() {}
};

TEST_F(_6502JumpsAndCallsTests, JSRCanJumpToSubroutine) {
    cpu.mem[0xFFFC] = CPU::INS_JSR;     //6 cycles
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0xFFFE] = 0x80;
    constexpr byte EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0x8000);
    EXPECT_EQ(cpu.SP, 0xFD);
    EXPECT_EQ(cpu.mem[0x1FF], 0xFF);
    EXPECT_EQ(cpu.mem[0x1FE], 0xFE);
}

TEST_F(_6502JumpsAndCallsTests, JSRDoesNotAffectTheProcessorStatus) {
    cpu.mem[0xFFFC] = CPU::INS_JSR;     //6 cycles
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0xFFFE] = 0x80;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cpu.PS, psCopy);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
}

TEST_F(_6502JumpsAndCallsTests, RTSCanReturnFromSubroutine) {
    cpu.PC = 0x8000;
    cpu.mem[0x1FF] = 0xFF;
    cpu.mem[0x1FE] = 0x02;
    cpu.SP = 0xFD;
    cpu.mem[0x8000] = CPU::INS_RTS; //6 cycles
    constexpr byte EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, 0xFF);
    EXPECT_EQ(cpu.PC, 0xFF03);
}

TEST_F(_6502JumpsAndCallsTests, RTSDoesNotAffectTheProcessorStatus) {
    cpu.PC = 0x8000;
    cpu.mem[0x1FF] = 0xFF;
    cpu.mem[0x1FE] = 0x02;
    cpu.SP = 0xFD;
    cpu.mem[0x8000] = CPU::INS_RTS; //6 cycles
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cpu.PS, psCopy);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
}

TEST_F(_6502JumpsAndCallsTests, JSRAndRTSCanJumpToAndReturnFromSubroutine) {
    cpu.mem[0xFFFC] = CPU::INS_JSR;     //6 cycles
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0xFFFE] = 0x80;
    cpu.mem[0x8000] = CPU::INS_LDA_IM;  //2 cycles
    cpu.mem[0x8001] = 0x42;
    cpu.mem[0x8002] = CPU::INS_RTS;     //6 cycles
    constexpr byte EXPECTED_CYCLES = 14;
    constexpr byte INSTRUCTIONS_TO_EXECUTE = 3;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES, INSTRUCTIONS_TO_EXECUTE);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.PC, 0xFFFF);
}

//TODO create tests for JMP in both address modes
/*
TEST_F(_6502JumpsAndCallsTests, JMPCanJumpToANewAddress) {
    cpu.mem[0xFFFC] = CPU::INS_JSR;     //6 cycles
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0xFFFE] = 0x80;
    cpu.mem[0x8000] = CPU::INS_LDA_IM;  //2 cycles
    cpu.mem[0x8001] = 0x42;
    cpu.mem[0x8002] = CPU::INS_RTS;     //6 cycles
    constexpr byte EXPECTED_CYCLES = 14;
    constexpr byte INSTRUCTIONS_TO_EXECUTE = 3;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES, INSTRUCTIONS_TO_EXECUTE);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.PC, 0xFFFF);
}*/
