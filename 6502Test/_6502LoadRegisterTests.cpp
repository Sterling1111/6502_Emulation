#include "gtest/gtest.h"
#include "6502.h"

using namespace m6502;

class _6502LoadRegisterTests : public testing::Test {
public:
    CPU cpu;
    virtual void SetUp() { cpu.reset(); }
    virtual void TearDown() {}

    void TestLoadRegisterImmediate(byte, byte CPU::*);
    void TestLoadRegisterZeroPage(byte, byte CPU::*);
    void TestLoadRegisterZeroPageX(byte, byte CPU::*);
    void TestLoadRegisterZeroPageXWhenItRaps(byte, byte CPU::*);
    void TestLoadRegisterAbsolute(byte, byte CPU::*);
    void TestLoadRegisterAbsoluteX(byte, byte CPU::*);
    void TestLoadRegisterAbsoluteY(byte, byte CPU::*);
    void TestLoadRegisterAbsoluteXWhenPageBounderyCrossed(byte, byte CPU::*);
    void TestLoadRegisterAbsoluteYWhenPageBounderyCrossed(byte, byte CPU::*);
};

static void VerifyUnmodifiedCPUFlagsFromLoadRegister(const std::bitset<CPU::StatusFlags::numFlags>& ps, const std::bitset<CPU::StatusFlags::numFlags>& psCopy) {
    EXPECT_EQ(ps.test(CPU::StatusFlags::C), psCopy.test(CPU::StatusFlags::C));
    EXPECT_EQ(ps.test(CPU::StatusFlags::I), psCopy.test(CPU::StatusFlags::I));
    EXPECT_EQ(ps.test(CPU::StatusFlags::D), psCopy.test(CPU::StatusFlags::D));
    EXPECT_EQ(ps.test(CPU::StatusFlags::B), psCopy.test(CPU::StatusFlags::B));
    EXPECT_EQ(ps.test(CPU::StatusFlags::V), psCopy.test(CPU::StatusFlags::V));
}

TEST_F(_6502LoadRegisterTests, CPUTerminatesIfInstructionInvalid) {
    cpu.mem[0xFFFC] = 0x00;
    cpu.mem[0xFFFD] = 0x00;
    constexpr dword EXPECTED_CYCLES = 1;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
}

TEST_F(_6502LoadRegisterTests, CPUCanCompleteCurrentInstructionIfInsufficientCyclesGiven) {
    cpu.mem[0xFFFC] = CPU::INS_LDA_IM;
    cpu.mem[0xFFFD] = 0x84;
    constexpr dword EXPECTED_CYCLES = 2;
    constexpr dword GIVEN_CYCLES = 2;
    dword cyclesUsed = cpu.execute(GIVEN_CYCLES);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
}

TEST_F(_6502LoadRegisterTests, CPUDoesNothingWhenWeExecuteZeroCycles) {
    constexpr dword EXPECTED_CYCLES = 0;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
}

TEST_F(_6502LoadRegisterTests, LDAImmediateCanAffectTheZeroFlag) {
    cpu.PS.set(CPU::StatusFlags::Z, false);
    cpu.mem[0xFFFC] = CPU::INS_LDA_IM;
    cpu.mem[0xFFFD] = 0x00;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 2;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.PS.test(CPU::StatusFlags::Z), true);
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterImmediate(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, false);
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x84;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 2;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x84);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_TRUE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterZeroPage(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0042] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 3;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterZeroPageX(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.X = 5;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0047] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterZeroPageXWhenItRaps(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0x007F] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsolute(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4480] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsoluteX(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.X = 1;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4481] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsoluteXWhenPageBounderyCrossed(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4401] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 5;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsoluteY(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.Y = 1;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4481] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

void _6502LoadRegisterTests::TestLoadRegisterAbsoluteYWhenPageBounderyCrossed(byte opcode, byte CPU::* Register) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4401] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 5;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAImmediateCanLoadAValueIntoTheARegister) {
    TestLoadRegisterImmediate(CPU::INS_LDA_IM, &CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDXImmediateCanLoadAValueIntoTheXRegister) {
    TestLoadRegisterImmediate(CPU::INS_LDX_IM, &CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDYImmediateCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterImmediate(CPU::INS_LDY_IM, &CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDAZeroPageCanLoadAValueIntoTheARegister) {
    TestLoadRegisterZeroPage(CPU::INS_LDA_ZP, &CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDXZeroPageCanLoadAValueIntoTheXRegister) {
    TestLoadRegisterZeroPage(CPU::INS_LDX_ZP, &CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDYZeroPageCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterZeroPage(CPU::INS_LDY_ZP, &CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDAZeroPageXCanLoadAValueIntoTheARegister) {
    TestLoadRegisterZeroPageX(CPU::INS_LDA_ZPX, &CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDAZeroPageXCanLoadAValueIntoTheARegisterWhenItRaps) {
    TestLoadRegisterZeroPageXWhenItRaps(CPU::INS_LDA_ZPX, &CPU::A );
}

TEST_F(_6502LoadRegisterTests, LDYZeroPageXCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterZeroPageX(CPU::INS_LDY_ZPX, &CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDYZeroPageXCanLoadAValueIntoTheYRegisterWhenItRaps) {
    TestLoadRegisterZeroPageXWhenItRaps(CPU::INS_LDY_ZPX, &CPU::Y );
}

TEST_F(_6502LoadRegisterTests, LDXZeroPageYCanLoadAValueIntoTheXRegister) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.Y = 5;
    cpu.mem[0xFFFC] = CPU::INS_LDX_ZPY;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0047] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.X, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDXZeroPageYCanLoadAValueIntoTheXRegisterWhenItRaps) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = CPU::INS_LDX_ZPY;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0x007F] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.X, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteCanLoadAValueIntoTheARegister) {
    TestLoadRegisterAbsolute(CPU::INS_LDA_ABS, &CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDXAbsoluteCanLoadAValueIntoTheXRegister) {
    TestLoadRegisterAbsolute(CPU::INS_LDX_ABS, &CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDYAbsoluteCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterAbsolute(CPU::INS_LDY_ABS, &CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteXCanLoadAValueIntoTheARegister) {
    TestLoadRegisterAbsoluteX(CPU::INS_LDA_ABSX, &CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteXCanLoadAValueIntoTheARegisterWhenItRaps) {
    TestLoadRegisterAbsoluteXWhenPageBounderyCrossed(CPU::INS_LDA_ABSX, &CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDYAbsoluteXCanLoadAValueIntoTheYRegister) {
    TestLoadRegisterAbsoluteX(CPU::INS_LDY_ABSX, &CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDYAbsoluteXCanLoadAValueIntoTheYSRegisterWhenItRaps) {
    TestLoadRegisterAbsoluteXWhenPageBounderyCrossed(CPU::INS_LDY_ABSX, &CPU::Y);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteYCanLoadAValueIntoTheARegister) {
    TestLoadRegisterAbsoluteY(CPU::INS_LDA_ABSY, &CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDAAbsoluteYCanLoadAValueIntoTheARegisterWhenItRaps) {
    TestLoadRegisterAbsoluteYWhenPageBounderyCrossed(CPU::INS_LDA_ABSY, &CPU::A);
}

TEST_F(_6502LoadRegisterTests, LDXAbsoluteYCanLoadAValueIntoTheXRegister) {
    TestLoadRegisterAbsoluteY(CPU::INS_LDX_ABSY, &CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDXAbsoluteYCanLoadAValueIntoTheXRegisterWhenItRaps) {
    TestLoadRegisterAbsoluteYWhenPageBounderyCrossed(CPU::INS_LDX_ABSY, &CPU::X);
}

TEST_F(_6502LoadRegisterTests, LDAXIndirectCanLoadAValueIntoTheARegister) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.X = 0x04;
    cpu.mem[0xFFFC] = CPU::INS_LDA_XIND;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0006] = 0x00;
    cpu.mem[0x0007] = 0x80;
    cpu.mem[0x8000] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAXIndirectCanLoadAValueIntoTheARegisterWhenItRaps) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = CPU::INS_LDA_XIND;
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0x00FF] = 0x00;
    cpu.mem[0x0000] = 0x80;
    cpu.mem[0x8000] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAIndirectYCanLoadAValueIntoTheARegister) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.Y = 0x04;
    cpu.mem[0xFFFC] = CPU::INS_LDA_INDY;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0002] = 0x00;
    cpu.mem[0x0003] = 0x80;
    cpu.mem[0x8004] = 0x37;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 5;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

TEST_F(_6502LoadRegisterTests, LDAIndirectYCanLoadAValueIntoTheARegisterWhenZeroPageCrossed) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
    cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = CPU::INS_LDA_INDY;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0002] = 0x02;
    cpu.mem[0x0003] = 0x80;
    cpu.mem[0x8101] = 0x37; //0x8002 + 0xFF
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute(EXPECTED_CYCLES);

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    VerifyUnmodifiedCPUFlagsFromLoadRegister(cpu.PS, psCopy);
}

