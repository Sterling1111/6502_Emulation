#include "gtest/gtest.h"
#include "6502.h"

class _6502LoadRegisterTests : public testing::Test {
public:
    m6502::CPU cpu;
    virtual void SetUp() { cpu.PC = 0xFFFC; }
    virtual void TearDown() {}

    void TestLoadRegisterImmediate(m6502::byte, m6502::byte m6502::CPU::*);
    void TestLoadRegisterZeroPage(m6502::byte, m6502::byte m6502::CPU::*);
    void TestLoadRegisterZeroPageX(m6502::byte, m6502::byte m6502::CPU::*);
    void TestLoadRegisterZeroPageXWhenItRaps(m6502::byte, m6502::byte m6502::CPU::*);
    void TestLoadRegisterAbsolute(m6502::byte, m6502::byte m6502::CPU::*);
    void TestLoadRegisterAbsoluteX(m6502::byte, m6502::byte m6502::CPU::*);
    void TestLoadRegisterAbsoluteY(m6502::byte, m6502::byte m6502::CPU::*);
    void TestLoadRegisterAbsoluteXWhenPageBounderyCrossed(m6502::byte, m6502::byte m6502::CPU::*);
    void TestLoadRegisterAbsoluteYWhenPageBounderyCrossed(m6502::byte, m6502::byte m6502::CPU::*);
};

static void VerifyUnmodifiedCPUFlagsFromLoadRegister(const std::bitset<m6502::CPU::StatusFlags::numFlags>& ps, const std::bitset<m6502::CPU::StatusFlags::numFlags>& psCopy) {
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::C), psCopy.test(m6502::CPU::StatusFlags::C));
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::I), psCopy.test(m6502::CPU::StatusFlags::I));
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::D), psCopy.test(m6502::CPU::StatusFlags::D));
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::B), psCopy.test(m6502::CPU::StatusFlags::B));
    EXPECT_EQ(ps.test(m6502::CPU::StatusFlags::V), psCopy.test(m6502::CPU::StatusFlags::V));
}

TEST_F(_6502LoadRegisterTests, CPUTerminatesIfInstructionInvalid) {
    cpu.mem[0xFFFC] = 0x00;
    cpu.mem[0xFFFD] = 0x00;
    constexpr m6502::dword EXPECTED_CYCLES = 1;
    m6502::dword cyclesUsed = cpu.execute();
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
}

TEST_F(_6502LoadRegisterTests, CPUDoesNothingWhenWeExecuteZeroInstructions) {
    constexpr m6502::dword INSTRUCTIONS = 0;
    constexpr m6502::dword EXPECTED_CYCLES = 0;
    m6502::dword cyclesUsed = cpu.execute(INSTRUCTIONS);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
}

TEST_F(_6502LoadRegisterTests, LDAImmediateCanAffectTheZeroFlag) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, false);
    cpu.mem[0xFFFC] = m6502::CPU::INS_LDA_IM;
    cpu.mem[0xFFFD] = 0x00;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 2;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PS.test(m6502::CPU::StatusFlags::Z), true);
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterImmediate(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, false);
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x84;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 2;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x84);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_TRUE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterZeroPage(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0042] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 3;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterZeroPageX(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.X = 5;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0047] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterZeroPageXWhenItRaps(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0x007F] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsolute(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4480] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsoluteX(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.X = 1;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4481] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsoluteXWhenPageBounderyCrossed(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4401] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 5;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsoluteY(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.Y = 1;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4481] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsoluteYWhenPageBounderyCrossed(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4401] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 5;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAImmediateCanLoadAValueIntoTheARegister) {
    TestLoadRegisterImmediate(m6502::CPU::INS_LDA_IM, &m6502::CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDXImmediateCanLoadAValueIntoTheXRegister) {
    TestLoadRegisterImmediate(m6502::CPU::INS_LDX_IM, &m6502::CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDYImmediateCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterImmediate(m6502::CPU::INS_LDY_IM, &m6502::CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDAZeroPageCanLoadAValueIntoTheARegister) {
    TestLoadRegisterZeroPage(m6502::CPU::INS_LDA_ZP, &m6502::CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDXZeroPageCanLoadAValueIntoTheXRegister) {
    TestLoadRegisterZeroPage(m6502::CPU::INS_LDX_ZP, &m6502::CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDYZeroPageCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterZeroPage(m6502::CPU::INS_LDY_ZP, &m6502::CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDAZeroPageXCanLoadAValueIntoTheARegister) {
    TestLoadRegisterZeroPageX(m6502::CPU::INS_LDA_ZPX, &m6502::CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDAZeroPageXCanLoadAValueIntoTheARegisterWhenItRaps) {
    TestLoadRegisterZeroPageXWhenItRaps(m6502::CPU::INS_LDA_ZPX, &m6502::CPU::A );
}

TEST_F(_6502LoadRegisterTests, LDYZeroPageXCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterZeroPageX(m6502::CPU::INS_LDY_ZPX, &m6502::CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDYZeroPageXCanLoadAValueIntoTheYRegisterWhenItRaps) {
    TestLoadRegisterZeroPageXWhenItRaps(m6502::CPU::INS_LDY_ZPX, &m6502::CPU::Y );
}

TEST_F(_6502LoadRegisterTests, LDXZeroPageYCanLoadAValueIntoTheXRegister) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.Y = 5;
    cpu.mem[0xFFFC] = m6502::CPU::INS_LDX_ZPY;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0047] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.X, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDXZeroPageYCanLoadAValueIntoTheXRegisterWhenItRaps) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = m6502::CPU::INS_LDX_ZPY;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0x007F] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 4;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.X, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteCanLoadAValueIntoTheARegister) {
    TestLoadRegisterAbsolute(m6502::CPU::INS_LDA_ABS, &m6502::CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDXAbsoluteCanLoadAValueIntoTheXRegister) {
    TestLoadRegisterAbsolute(m6502::CPU::INS_LDX_ABS, &m6502::CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDYAbsoluteCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterAbsolute(m6502::CPU::INS_LDY_ABS, &m6502::CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteXCanLoadAValueIntoTheARegister) {
    TestLoadRegisterAbsoluteX(m6502::CPU::INS_LDA_ABSX, &m6502::CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteXCanLoadAValueIntoTheARegisterWhenItRaps) {
    TestLoadRegisterAbsoluteXWhenPageBounderyCrossed(m6502::CPU::INS_LDA_ABSX, &m6502::CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDYAbsoluteXCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterAbsoluteX(m6502::CPU::INS_LDY_ABSX, &m6502::CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDYAbsoluteXCanLoadAValueIntoTheYSRegisterWhenItRaps) {
    TestLoadRegisterAbsoluteXWhenPageBounderyCrossed(m6502::CPU::INS_LDY_ABSX, &m6502::CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteYCanLoadAValueIntoTheARegister) {
    TestLoadRegisterAbsoluteY(m6502::CPU::INS_LDA_ABSY, &m6502::CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteYCanLoadAValueIntoTheARegisterWhenItRaps) {
    TestLoadRegisterAbsoluteYWhenPageBounderyCrossed(m6502::CPU::INS_LDA_ABSY, &m6502::CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDXAbsoluteYCanLoadAValueIntoTheXRegister) {
    TestLoadRegisterAbsoluteY(m6502::CPU::INS_LDX_ABSY, &m6502::CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDXAbsoluteYCanLoadAValueIntoTheXRegisterWhenItRaps) {
    TestLoadRegisterAbsoluteYWhenPageBounderyCrossed(m6502::CPU::INS_LDX_ABSY, &m6502::CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDAXIndirectCanLoadAValueIntoTheARegister) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.X = 0x04;
    cpu.mem[0xFFFC] = m6502::CPU::INS_LDA_XIND;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0006] = 0x00;
    cpu.mem[0x0007] = 0x80;
    cpu.mem[0x8000] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAXIndirectCanLoadAValueIntoTheARegisterWhenItRaps) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = m6502::CPU::INS_LDA_XIND;
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0x00FF] = 0x00;
    cpu.mem[0x0000] = 0x80;
    cpu.mem[0x8000] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAIndirectYCanLoadAValueIntoTheARegister) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.Y = 0x04;
    cpu.mem[0xFFFC] = m6502::CPU::INS_LDA_INDY;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0002] = 0x00;
    cpu.mem[0x0003] = 0x80;
    cpu.mem[0x8004] = 0x37;
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 5;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAIndirectYCanLoadAValueIntoTheARegisterWhenZeroPageCrossed) {
    cpu.PS.set(m6502::CPU::StatusFlags::Z, true);
    cpu.PS.set(m6502::CPU::StatusFlags::N, true);
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = m6502::CPU::INS_LDA_INDY;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0002] = 0x02;
    cpu.mem[0x0003] = 0x80;
    cpu.mem[0x8101] = 0x37; //0x8002 + 0xFF
    auto psCopy = cpu.PS;
    constexpr m6502::dword EXPECTED_CYCLES = 6;
    m6502::dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(m6502::CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

