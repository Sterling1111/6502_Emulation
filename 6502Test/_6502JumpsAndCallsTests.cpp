#include "gtest/gtest.h"
#include "6502.h"

class _6502JumpsAndCallsTests : public testing::Test {
public:
    m6502::CPU cpu{.0001};
    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}
};

TEST_F(_6502JumpsAndCallsTests, JSRCanJumpToSubroutine) {
    cpu.mem[0xFFFC] = m6502::CPU::INS_JSR;     //6 cycles
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0xFFFE] = 0x80;
    auto psCopy = cpu.PS;
    constexpr m6502::byte EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0x8000);
    EXPECT_EQ(cpu.SP, 0xFD);
    EXPECT_EQ(cpu.mem[0x1FF], 0xFF);
    EXPECT_EQ(cpu.mem[0x1FE], 0xFE);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502JumpsAndCallsTests, RTSCanReturnFromSubroutine) {
    cpu.PC = 0x8000;
    cpu.mem[0x1FF] = 0xFF;
    cpu.mem[0x1FE] = 0x02;
    cpu.SP = 0xFD;
    cpu.mem[0x8000] = m6502::CPU::INS_RTS;
    auto psCopy = cpu.PS;
    constexpr m6502::byte EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, 0xFF);
    EXPECT_EQ(cpu.PC, 0xFF03);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502JumpsAndCallsTests, JSRAndRTSCanJumpToAndReturnFromSubroutine) {
    cpu.mem[0xFFFC] = m6502::CPU::INS_JSR;     //6 cycles
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0xFFFE] = 0x80;
    cpu.mem[0x8000] = m6502::CPU::INS_LDA_IM;  //2 cycles
    cpu.mem[0x8001] = 0x42;
    cpu.mem[0x8002] = m6502::CPU::INS_RTS;     //6 cycles
    auto psCopy = cpu.PS;
    constexpr m6502::byte EXPECTED_CYCLES = 14;
    constexpr m6502::byte INSTRUCTIONS = 3;
    m6502::dword cyclesUsed = cpu.execute(INSTRUCTIONS);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.PC, 0xFFFF);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502JumpsAndCallsTests, JMPAbsoluteCanJumpToANewAddress) {
    cpu.mem[0xFFFC] = m6502::CPU::INS_JMP_ABS;
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0xFFFE] = 0x80;
    auto psCopy = cpu.PS;
    constexpr m6502::byte EXPECTED_CYCLES = 3;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0x8000);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502JumpsAndCallsTests, JMPIndirectCanJumpToANewAddress) {
    cpu.mem[0xFFFC] = m6502::CPU::INS_JMP_IND;
    cpu.mem[0xFFFD] = 0x20;
    cpu.mem[0xFFFE] = 0x01;
    cpu.mem[0x0120] = 0xFC;
    cpu.mem[0x0121] = 0xBA;
    auto psCopy = cpu.PS;
    constexpr m6502::byte EXPECTED_CYCLES = 5;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0xBAFC);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502JumpsAndCallsTests, JMPIndirectCanJumpToANewAddressOnPageBoundery) {
    cpu.mem[0xFFFC] = m6502::CPU::INS_JMP_IND;
    cpu.mem[0xFFFD] = 0xFF;
    cpu.mem[0xFFFE] = 0x01;
    cpu.mem[0x01FF] = 0xFC;
    cpu.mem[0x0100] = 0xBA;
    auto psCopy = cpu.PS;
    constexpr m6502::byte EXPECTED_CYCLES = 5;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PC, 0xBAFC);
    EXPECT_EQ(cpu.PS, psCopy);
}
