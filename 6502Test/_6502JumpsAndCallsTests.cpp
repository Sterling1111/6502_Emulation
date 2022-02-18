#include "gtest/gtest.h"
#include "6502.h"

using namespace m6502;

class _6502JumpsAndCallsTests : public testing::Test {
public:
    CPU cpu;
    virtual void SetUp() { cpu.reset(); }
    virtual void TearDown() {}
};

static void VerifyUnmodifiedCPUFlagsFromJumpsAndCalls(const CPU::PS& ps, const CPU::PS& psCopy) {
    EXPECT_EQ(ps.N, psCopy.N);
    EXPECT_EQ(ps.Z, psCopy.Z);
    EXPECT_EQ(ps.C, psCopy.C);
    EXPECT_EQ(ps.I, psCopy.I);
    EXPECT_EQ(ps.D, psCopy.D);
    EXPECT_EQ(ps.B, psCopy.B);
    EXPECT_EQ(ps.V, psCopy.V);
}

TEST_F(_6502JumpsAndCallsTests, CanJumpToSubroutine) {
    cpu.mem[0xFFFC] = CPU::INS_JSR;     //6 cycles
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0xFFFE] = 0x80;
    CPU::PS psCopy = cpu.ps;
    constexpr byte EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    VerifyUnmodifiedCPUFlagsFromJumpsAndCalls(cpu.ps, psCopy);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0x8000);
    EXPECT_EQ(cpu.SP, 0xFD);
    EXPECT_EQ(cpu.mem[0x1FF], 0xFF);
    EXPECT_EQ(cpu.mem[0x1FE], 0xFE);
}

TEST_F(_6502JumpsAndCallsTests, CanReturnFromSubroutine) {
    cpu.PC = 0x8000;
    cpu.mem[0x1FF] = 0xFF;
    cpu.mem[0x1FE] = 0x02;
    cpu.SP = 0xFD;
    cpu.mem[0x8000] = CPU::INS_RTS; //6 cycles
    CPU::PS psCopy = cpu.ps;
    constexpr byte EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    VerifyUnmodifiedCPUFlagsFromJumpsAndCalls(cpu.ps, psCopy);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, 0xFF);
    EXPECT_EQ(cpu.PC, 0xFF03);
}

TEST_F(_6502JumpsAndCallsTests, CanJumpToAndReturnFromSubroutine) {
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