#include "gtest/gtest.h"
#include "6502.h"
#include <functional>

class _6502LogicalOperationTests : public testing::Test {
public:
    m6502::CPU cpu{.0001};
    virtual void SetUp() { setCPUState(); }
    virtual void TearDown() {}

    void setCPUState() {
        cpu.reset();
        cpu.PC = 0xFFFC;
    }

    void TestLogicalOperationImmediate(m6502::byte, m6502::byte, m6502::byte byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationZeroPage(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationZeroPageX(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationZeroPageXWhenItRaps(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)> &);
    void TestLogicalOperationAbsolute(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationAbsoluteX(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationAbsoluteY(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationAbsoluteXWhenPageBounderyCrossed(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationAbsoluteYWhenPageBounderyCrossed(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationIndirectY(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationIndirectYWhenPageBounderyCrossed(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationXIndirect(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestLogicalOperationXIndirectWhenItRaps(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&);
    void TestBitOperationZeroPage(m6502::byte, m6502::byte, m6502::byte);
    void TestBitOperationAbsolute(m6502::byte, m6502::byte, m6502::byte);

    static void invokeTestFixtureCasesAND(m6502::byte, const std::function<void(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&)>&);
    static void invokeTestFixtureCasesEOR(m6502::byte, const std::function<void(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&)>&);
    static void invokeTestFixtureCasesORA(m6502::byte, const std::function<void(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&)>&);
};

static void VerifyUnmodifiedCPUFlagsFromLogicalOperation(const std::bitset<m6502::CPU::StatusFlags::numFlags>& ps, const std::bitset<m6502::CPU::StatusFlags::numFlags>& psCopy) {
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::C), psCopy.test(m6502::CPU::StatusFlags::C));
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::I), psCopy.test(m6502::CPU::StatusFlags::I));
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::D), psCopy.test(m6502::CPU::StatusFlags::D));
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::B), psCopy.test(m6502::CPU::StatusFlags::B));
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::V), psCopy.test(m6502::CPU::StatusFlags::V));
}

void _6502LogicalOperationTests::TestLogicalOperationImmediate(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = value;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 2;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(valueA, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, valueA));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, valueA) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationZeroPage(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0042] = value;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 3;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(value, valueA));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, valueA));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, valueA) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationZeroPageX(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.X = 5;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0047] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationZeroPageXWhenItRaps(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0x007F] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationAbsolute(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4480] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationAbsoluteX(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.X = 1;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4481] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationAbsoluteXWhenPageBounderyCrossed(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4401] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 5;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationAbsoluteY(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.Y = 1;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4481] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationAbsoluteYWhenPageBounderyCrossed(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4401] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 5;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationIndirectY(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.Y = 0x04;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0002] = 0x00;
    cpu.mem[0x0003] = 0x80;
    cpu.mem[0x8004] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 5;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationIndirectYWhenPageBounderyCrossed(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0002] = 0x02;
    cpu.mem[0x0003] = 0x80;
    cpu.mem[0x8101] = value; //0x8002 + 0xFF
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationXIndirect(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.X = 0x04;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0006] = 0x00;
    cpu.mem[0x0007] = 0x80;
    cpu.mem[0x8000] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

void _6502LogicalOperationTests::TestLogicalOperationXIndirectWhenItRaps(m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, op(value, cpu.A));
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(op(value, cpu.A) & 0x80));
    cpu.A = valueA;
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0x00FF] = 0x00;
    cpu.mem[0x0000] = 0x80;
    cpu.mem[0x8000] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}

/*
void _6502LogicalOperationTests::TestBitOperationZeroPage(m6502::byte opcode, m6502::byte valueA, m6502::byte value) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, value & cpu.A);
    cpu.PS.set(m6502::CPU::StatusFlags::N, !(value & cpu.A & 0x80));
    cpu.PS.set(m6502::CPU::StatusFlags::V, !(value & cpu.A & 0x40));
    cpu.A = valueA;
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0x00FF] = 0x00;
    cpu.mem[0x0000] = 0x80;
    cpu.mem[0x8000] = value;
    auto psCopy = cpu.PS;
    auto accumCopy = cpu.A;
    constexpr m6502::dword EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, op(accumCopy, value));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), !op(value, accumCopy));
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::N), (op(value, accumCopy) & 0x80) >> 7);
    VerifyUnmodifiedCPUFlagsFromLogicalOperation(cpu.PS, psCopy);
    setCPUState();
}
*/

void _6502LogicalOperationTests::invokeTestFixtureCasesAND(m6502::byte opcode, const std::function<void(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&)>& testAND) {
    testAND(opcode, 0b01010101, 0x0F, std::bit_and<>());  //neither 0 nor negative
    testAND(opcode, 0b00000000, 0xFF, std::bit_and<>());  //0
    testAND(opcode, 0b10000000, 0xFF, std::bit_and<>());  //negative
}

void _6502LogicalOperationTests::invokeTestFixtureCasesEOR(m6502::byte opcode, const std::function<void(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&)>& testEOR) {
    testEOR(opcode, 0b01010101, 0x0F, std::bit_xor<>());    //neither 0 nor negative
    testEOR(opcode, 0b00000000, 0x7F, std::bit_xor<>());    //0
    testEOR(opcode, 0b01010101, 0x8F, std::bit_xor<>());    //negative
}

void _6502LogicalOperationTests::invokeTestFixtureCasesORA(m6502::byte opcode, const std::function<void(m6502::byte, m6502::byte, m6502::byte, const std::function<m6502::byte(m6502::byte, m6502::byte)>&)>& testORA) {
    testORA(opcode, 0b01010101, 0x0F, std::bit_or<>());     //neither 0 nor negative
    testORA(opcode, 0b00000000, 0x00, std::bit_or<>());     //0
    testORA(opcode, 0b11010101, 0x8F, std::bit_or<>());     //negative
}

TEST_F(_6502LogicalOperationTests, ANDImmediateCanANDAValueIntoARegister) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_IM, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationImmediate(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORImmediateCanEORAValueIntoARegister) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_IM, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationImmediate(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAImmediateCanORAValueIntoARegister) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_IM, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationImmediate(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDZeroPageCanANDAValueIntoARegister) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_ZP, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPage(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORZeroPageCanEORAValueIntoARegister) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_ZP, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPage(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAZeroPageCanORAValueIntoARegister) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_ZP, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPage(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDZeroPageXCanANDAValueIntoARegister) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_ZPX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPageX(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORZeroPageXCanEORAValueIntoARegister) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_ZPX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPageX(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAZeroPageXCanORAValueIntoARegister) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_ZPX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPageX(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDZeroPageXCanANDAValueIntoARegisterWhenItRaps) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_ZPX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPageXWhenItRaps(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORZeroPageXCanEORAValueIntoARegisterWhenItRaps) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_ZPX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPageXWhenItRaps(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAZeroPageXCanORAValueIntoARegisterWhenItRaps) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_ZPX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationZeroPageXWhenItRaps(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDAbsoluteCanANDAValueIntoARegister) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_ABS, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsolute(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORAbsoluteCanEORAValueIntoARegister) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_ABS, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsolute(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAAbsoluteCanORAAValueIntoARegister) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_ABS, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsolute(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDAbsoluteXCanANDAValueIntoARegister) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_ABSX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteX(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDAbsoluteXCanANDAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_ABSX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteXWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORAbsoluteXCanEORAValueIntoARegister) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_ABSX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteX(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORAbsoluteXCanEORAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_ABSX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteXWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAAbsoluteXCanORAValueIntoARegister) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_ABSX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteX(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAAbsoluteXCanORAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_ABSX, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteXWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDAbsoluteYCanANDAValueIntoARegister) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_ABSY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteY(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDAbsoluteYCanANDAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_ABSY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteYWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORAbsoluteYCanEORAValueIntoARegister) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_ABSY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteY(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORAbsoluteYCanEORAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_ABSY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteYWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAAbsoluteYCanORAValueIntoARegister) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_ABSY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteY(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAAbsoluteYCanORAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_ABSY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationAbsoluteYWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDIndirectYCanANDAValueIntoARegister) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_INDY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationIndirectY(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDIndirectYCanANDAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_INDY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationIndirectYWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORIndirectYCanEORAValueIntoARegister) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_INDY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationIndirectY(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORIndirectYCanEORAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_INDY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationIndirectYWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAIndirectYCanORAValueIntoARegister) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_INDY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationIndirectY(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAIndirectYCanORAValueIntoARegisterWhenPageBounderyCrossed) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_INDY, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationIndirectYWhenPageBounderyCrossed(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDXIndirectCanANDAValueIntoARegister) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_XIND, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationXIndirect(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ANDXIndirectCanANDAValueIntoARegisterWhenItRaps) {
    invokeTestFixtureCasesAND(m6502::CPU::INS_AND_XIND, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationXIndirectWhenItRaps(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORXIndirectCanEORAValueIntoARegister) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_XIND, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationXIndirect(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, EORXIndirectCanEORAValueIntoARegisterWhenItRaps) {
    invokeTestFixtureCasesEOR(m6502::CPU::INS_EOR_XIND, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationXIndirectWhenItRaps(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAXIndirectCanORAValueIntoARegister) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_XIND, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationXIndirect(opcode, valueA, value, op);
    });
}

TEST_F(_6502LogicalOperationTests, ORAXIndirectCanORAValueIntoARegisterWhenItRaps) {
    invokeTestFixtureCasesORA(m6502::CPU::INS_ORA_XIND, [this](m6502::byte opcode, m6502::byte valueA, m6502::byte value, const std::function<m6502::byte(m6502::byte, m6502::byte)>& op) {
        TestLogicalOperationXIndirectWhenItRaps(opcode, valueA, value, op);
    });
}

//TEST_F(_6502LogicalOperationTests, )






