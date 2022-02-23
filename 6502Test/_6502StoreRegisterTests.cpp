#include "gtest/gtest.h"
#include "6502.h"

using namespace m6502;

class _6502StoreRegisterTests : public testing::Test {
public:
    CPU cpu;
    virtual void SetUp() { cpu.PC = 0xFFFC; }
    virtual void TearDown() {}

    void TestStoreRegisterZeroPage(byte, byte CPU::*);
    void TestStoreRegisterZeroPageX(byte, byte CPU::*);
    void TestStoreRegisterZeroPageXWhenItRaps(byte, byte CPU::*);
    void TestStoreRegisterAbsolute(byte, byte CPU::*);
};

void _6502StoreRegisterTests::TestStoreRegisterZeroPage(byte opcode, byte CPU::* Register) {
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0042] = cpu.*Register + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 3;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, cpu.mem[0x0042]);
    EXPECT_EQ(cpu.PS, psCopy);
}

void _6502StoreRegisterTests::TestStoreRegisterZeroPageX(byte opcode, byte CPU::* Register) {
    cpu.X = 5;
    cpu.*Register = 0x69;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0047] = cpu.*Register + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, cpu.mem[0x0047]);
    EXPECT_EQ(cpu.PS, psCopy);
}

void _6502StoreRegisterTests::TestStoreRegisterZeroPageXWhenItRaps(byte opcode, byte CPU::* Register) {
    cpu.X = 0xFF;
    cpu.*Register = 0x69;
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0x007F] = cpu.*Register + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, cpu.mem[0x007F]);
    EXPECT_EQ(cpu.PS, psCopy);
}

void _6502StoreRegisterTests::TestStoreRegisterAbsolute(byte opcode, byte CPU::* Register) {
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4480] = cpu.*Register + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.*Register, cpu.mem[0x4480]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAZeroPageCanStoreARegisterIntoMemory) {
    TestStoreRegisterZeroPage(CPU::INS_STA_ZP, &CPU::A);
}

TEST_F(_6502StoreRegisterTests, STXZeroPageCanStoreXRegisterIntoMemory) {
    TestStoreRegisterZeroPage(CPU::INS_STX_ZP, &CPU::X);
}

TEST_F(_6502StoreRegisterTests, STYZeroPageCanStoreYRegisterIntoMemory) {
    TestStoreRegisterZeroPage(CPU::INS_STY_ZP, &CPU::Y);
}

TEST_F(_6502StoreRegisterTests, STAZeroPageXCanStoreARegisterIntoMemory) {
    TestStoreRegisterZeroPageX(CPU::INS_STA_ZPX, &CPU::A);
}

TEST_F(_6502StoreRegisterTests, STAZeroPageXCanStoreARegisterIntoMemoryWhenItRaps) {
    TestStoreRegisterZeroPageXWhenItRaps(CPU::INS_STA_ZPX, &CPU::A);
}

TEST_F(_6502StoreRegisterTests, STYZeroPageXCanStoreYRegisterIntoMemory) {
    TestStoreRegisterZeroPageX(CPU::INS_STY_ZPX, &CPU::Y);
}

TEST_F(_6502StoreRegisterTests, STYZeroPageXCanStoreYRegisterIntoMemoryWhenItRaps) {
    TestStoreRegisterZeroPageXWhenItRaps(CPU::INS_STY_ZPX, &CPU::Y);
}

TEST_F(_6502StoreRegisterTests, STXZeroPageYCanStoreXRegisterIntoMemory) {
    cpu.Y = 5;
    cpu.mem[0xFFFC] = CPU::INS_STX_ZPY;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0047] = cpu.X + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.X, cpu.mem[0x0047]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STXZeroPageYCanStoreXRegisterIntoMemoryWhenItRaps) {
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = CPU::INS_STX_ZPY;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0x007F] = cpu.X + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 4;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.X, cpu.mem[0x007F]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAAbsoluteCanStoreARegisterIntoMemory) {
    TestStoreRegisterAbsolute(CPU::INS_STA_ABS, &CPU::A);
}

TEST_F(_6502StoreRegisterTests, STXAbsoluteCanStoreXRegisterIntoMemory) {
    TestStoreRegisterAbsolute(CPU::INS_STX_ABS, &CPU::X);
}

TEST_F(_6502StoreRegisterTests, STYAbsoluteCanStoreYRegisterIntoMemory) {
    TestStoreRegisterAbsolute(CPU::INS_STY_ABS, &CPU::Y);
}

TEST_F(_6502StoreRegisterTests, STAAbsoluteXCanStoreARegisterIntoMemory) {
    cpu.X = 1;
    cpu.mem[0xFFFC] = CPU::INS_STA_ABSX;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4481] = cpu.A + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 5;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, cpu.mem[0x4481]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAAbsoluteXCanStoreARegisterIntoMemoryWhenPageBounderyCrossed) {
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = CPU::INS_STA_ABSX;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4401] = cpu.A + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 5;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, cpu.mem[0x4401]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAAbsoluteYCanStoreARegisterIntoMemory) {
    cpu.Y = 1;
    cpu.mem[0xFFFC] = CPU::INS_STA_ABSY;
    cpu.mem[0xFFFD] = 0x80;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4481] = cpu.A + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 5;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, cpu.mem[0x4481]);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAAbsoluteYCanStoreARegisterIntoMemoryWhenPageBounderyCrossed) {
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = CPU::INS_STA_ABSX;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0xFFFE] = 0x44;
    cpu.mem[0x4401] = cpu.A + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 5;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, cpu.mem[0x4401]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAXIndirectCanStoreARegisterIntoMemory) {
    cpu.X = 0x04;
    cpu.mem[0xFFFC] = CPU::INS_STA_XIND;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0006] = 0x00;
    cpu.mem[0x0007] = 0x80;
    cpu.mem[0x8000] = cpu.A + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, cpu.mem[0x8000]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAXIndirectCanStoreARegisterIntoMemoryWhenItRaps) {
    cpu.PS.set(CPU::StatusFlags::Z, true);
cpu.PS.set(CPU::StatusFlags::N, true);
    cpu.X = 0xFF;
    cpu.mem[0xFFFC] = CPU::INS_STA_XIND;
    cpu.mem[0xFFFD] = 0x00;
    cpu.mem[0x00FF] = 0x00;
    cpu.mem[0x0000] = 0x80;
    cpu.mem[0x8000] = cpu.A + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, cpu.mem[0x8000]);
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAIndirectYCanStoreARegisterIntoMemory) {
    cpu.Y = 0x04;
    cpu.mem[0xFFFC] = CPU::INS_STA_INDY;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0002] = 0x00;
    cpu.mem[0x0003] = 0x80;
    cpu.mem[0x8004] = cpu.A + 1;
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, cpu.mem[0x8004]);
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::Z));
    EXPECT_FALSE(cpu.PS.test(CPU::StatusFlags::N));
    EXPECT_EQ(cpu.PS, psCopy);
}

TEST_F(_6502StoreRegisterTests, STAIndirectYCanStoreARegisterIntoMemoryWhenPageBounderyCrossed) {
    cpu.Y = 0xFF;
    cpu.mem[0xFFFC] = CPU::INS_STA_INDY;
    cpu.mem[0xFFFD] = 0x02;
    cpu.mem[0x0002] = 0x02;
    cpu.mem[0x0003] = 0x80;
    cpu.mem[0x8101] = cpu.A + 1; //0x8002 + 0xFF
    auto psCopy = cpu.PS;
    constexpr dword EXPECTED_CYCLES = 6;
    dword cyclesUsed = cpu.execute();

    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_EQ(cpu.A, cpu.mem[0x8101]);
    EXPECT_EQ(cpu.PS, psCopy);
}