#include "gtest/gtest.h"
#include "6502.h"

using namespace m6502;

class _6502StackOperationTests : public testing::Test {
public:
    CPU cpu;
    virtual void SetUp() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    virtual void TearDown() {}
};

static void VerifyUnmodifiedCPUFlagsFromLoadRegister(const std::bitset<CPU::StatusFlags::numFlags>& ps, const std::bitset<CPU::StatusFlags::numFlags>& psCopy) {
    EXPECT_EQ(ps.test(CPU::StatusFlags::C), psCopy.test(CPU::StatusFlags::C));
    EXPECT_EQ(ps.test(CPU::StatusFlags::I), psCopy.test(CPU::StatusFlags::I));
    EXPECT_EQ(ps.test(CPU::StatusFlags::D), psCopy.test(CPU::StatusFlags::D));
    EXPECT_EQ(ps.test(CPU::StatusFlags::B), psCopy.test(CPU::StatusFlags::B));
    EXPECT_EQ(ps.test(CPU::StatusFlags::U), psCopy.test(CPU::StatusFlags::U));
    EXPECT_EQ(ps.test(CPU::StatusFlags::V), psCopy.test(CPU::StatusFlags::V));
}

TEST_F(_6502StackOperationTests, TXSCanTransferXToStackPointer) {
    cpu.X = 0x42;
    cpu.mem[0xFFFC] = CPU::INS_TXS_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 2;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, cpu.X);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, TSXCanTransferXToStackPointer) {
    cpu.SP = 0x42;
    cpu.mem[0xFFFC] = CPU::INS_TSX_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 2;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, cpu.X);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, TSXCanSetTheZeroFlag) {
    cpu.SP = 0x00;
    cpu.mem[0xFFFC] = CPU::INS_TSX_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 2;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, cpu.X);
    EXPECT_TRUE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, TSXCanSetTheNegativeFlag) {
    cpu.SP = 0b10000000;
    cpu.mem[0xFFFC] = CPU::INS_TSX_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 2;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.SP, cpu.X);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_TRUE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, PHACanPushAccumulatorOnStack) {
    cpu.mem[0xFFFC] = CPU::INS_PHA_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 3;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.mem[0x01FF], cpu.A);
    EXPECT_EQ(cpu.SP, 0x00FE);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, PHPCanPushPSOnStack) {
    cpu.mem[0xFFFC] = CPU::INS_PHP_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 3;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.mem[0x01FF], static_cast<byte>(cpu.PS.to_ulong()));
    EXPECT_EQ(cpu.SP, 0x00FE);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, PLACanPullAccumulatorFromStack) {
    cpu.mem[0x01FF] = 0x42;
    cpu.SP = 0xFE;
    cpu.mem[0xFFFC] = CPU::INS_PLA_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.SP, 0x00FF);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, PLACanSetTheZeroFlag) {
    cpu.mem[0x01FF] = 0x00;
    cpu.SP = 0xFE;
    cpu.mem[0xFFFC] = CPU::INS_PLA_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.SP, 0x00FF);
    EXPECT_TRUE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, PLPCanPullPSFromStack) {
    cpu.mem[0x01FF] = 0x42;
    cpu.SP = 0xFE;
    cpu.mem[0xFFFC] = CPU::INS_PLP_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(static_cast<byte>(cpu.PS.to_ulong()), 0x42);
    EXPECT_EQ(cpu.SP, 0x00FF);
}

TEST_F(_6502StackOperationTests, PLPCanSetTheNegativeFlag) {
    cpu.mem[0x01FF] = 0b10000000;
    cpu.SP = 0xFE;
    cpu.mem[0xFFFC] = CPU::INS_PLA_IMP;
    auto psCopy = cpu.PS;
    constexpr byte EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0b10000000);
    EXPECT_EQ(cpu.SP, 0x00FF);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_TRUE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502StackOperationTests, PLPSetsStatusFlagsFromStack) {
    dword cyclesUsed{};
    bool diff{false};
    for (byte i{0}; i < 0xFF; ++i) {
        cpu.mem[0x01FF] = i;
        cpu.SP = 0xFE;
        cpu.mem[0xFFFC] = CPU::INS_PLP_IMP;
        cyclesUsed += cpu.execute();
        //only non-zero if they are different
        diff |= static_cast<byte>(cpu.PS.to_ulong()) xor i;
        cpu.reset();
        cpu.PC = 0xFFFC;
    }
    EXPECT_FALSE(diff);
}