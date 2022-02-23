#ifndef INC_6502_EMULATION_6502_H
#define INC_6502_EMULATION_6502_H

#include <iostream>
#include <chrono>
#include <bitset>

/*
 * immediate
 * zero page
 * zero page x
 * absolute
 * absolute x
 * absolute y
 * indirect x
 * indirect y
 */

namespace m6502 {
    typedef uint8_t byte;
    typedef uint16_t word;
    typedef uint32_t dword;
    typedef int32_t sdword;

    struct CPU;
}

struct m6502::CPU {
    word PC;    //program counter
    byte SP;    //stack pointer
    byte A, X, Y;   //registers

    static dword cycleDuration;

    /*carry, zero, interrupt disable, decimal mode, break command,
     * unused-always 1, overflow, negative*/
    enum StatusFlags {C, Z, I, D, B, U, V, N, numFlags};
    std::bitset<StatusFlags::numFlags> PS;

    struct Mem {
        static constexpr dword MAX_MEM = 1024 * 64;
        byte data[MAX_MEM];

        Mem() {initialize();}
        void initialize() { for (dword i{0}; i < MAX_MEM; i++) data[i] = 0; }
        byte operator[](dword address) const { return data[address]; }
        byte& operator[](dword address) { return data[address]; }
    };
    Mem mem;

    struct Cycles {
        explicit Cycles(double Mhz = 1) : cycles{0} {
            cycleDuration = getTCSFrequency() / Mhz;
        };
        static dword getTCSFrequency() {
            int eax{};
            __asm__("mov $0x16, %eax\n\t");
            __asm__("cpuid\n\t");
            __asm__("mov %%eax, %0\n\t":"=r" (eax));
            return eax;
        }
        Cycles&  operator++(){
            ++cycles;
            //busy wait. There is no other way.
            while((__builtin_ia32_rdtsc() - startTimePoint) < cycleDuration);
            startTimePoint = __builtin_ia32_rdtsc();
            return *this;
        }
        Cycles& operator+=(sdword num) {
            for (int i = 0; i < num; ++i) {
                this->operator++();
            }
            return *this;
        }
        bool operator> (sdword other) const {return cycles > other;}
        void reset() {
            cycles = 0;
            startTimePoint = __builtin_ia32_rdtsc();
        }
        sdword getCycles() const {return cycles;}
    private:
        sdword cycles;
        uint64_t startTimePoint;
        uint64_t cycleDuration;
    };
    Cycles cycles;

    struct CyclesIncrementer {
        Cycles& cycles;
        explicit CyclesIncrementer(Cycles& cycles) : cycles{cycles} {};
        ~CyclesIncrementer() { ++cycles; }
    };

    //instruction opcodes
    static constexpr byte
    //Load/Store Operations
    INS_LDA_IM = 0xA9,  INS_LDA_ZP = 0xA5, INS_LDA_ZPX = 0xB5, INS_LDA_ABS = 0xAD, INS_LDA_ABSX = 0xBD, INS_LDA_ABSY = 0xB9, INS_LDA_XIND = 0xA1, INS_LDA_INDY = 0xB1,
    INS_LDX_IM = 0xA2,  INS_LDX_ZP = 0xA6, INS_LDX_ZPY = 0xB6, INS_LDX_ABS = 0xAE, INS_LDX_ABSY = 0xBE,
    INS_LDY_IM = 0xA0,  INS_LDY_ZP = 0xA4, INS_LDY_ZPX = 0xB4, INS_LDY_ABS = 0xAC, INS_LDY_ABSX = 0xBC,
    INS_STA_ZP = 0x85, INS_STA_ZPX = 0x95, INS_STA_ABS = 0x8D, INS_STA_ABSX = 0x9D, INS_STA_ABSY = 0x99, INS_STA_XIND = 0x81, INS_STA_INDY = 0x91,
    INS_STX_ZP = 0x86, INS_STX_ZPY = 0x96, INS_STX_ABS = 0x8E,
    INS_STY_ZP = 0x84, INS_STY_ZPX = 0x94, INS_STY_ABS = 0x8C,
    //JUmps and Calls
    INS_RTS = 0x60,
    INS_JMP_ABS = 0x4C, INS_JMP_IND = 0x6C,
    INS_JSR = 0x20,
    //Stack Operations
    INS_PHA_IMP = 0x48,
    INS_PHP_IMP = 0x08,
    INS_PLA_IMP = 0x68,
    INS_PLP_IMP = 0x28,
    INS_TSX_IMP = 0xBA,
    INS_TXS_IMP = 0x9A,
    //Register Transfers
    INS_TXA_IMP = 0x8A,
    INS_TAX_IMP = 0xAA,
    INS_TAY_IMP = 0xA8;

    explicit CPU() {
        reset();
    };
    void reset();
    word readWord(word address);
    byte readByte(word address);
    byte fetchByte();
    word fetchWord();
    void writeWord(word data, word address);
    void writeByte(byte data, word address);
    void loadRegisterSetStatus(byte Register);
    dword execute(uint64_t instructionsToExecute = 1);
    //read instructions which return the byte in memory at the address for the given addressing mode
    inline byte readAddrZeroPage();
    inline byte readAddrZeroPageX();
    inline byte readAddrAbsolute();
    inline byte readAddrAbsoluteX();
    inline byte readAddrAbsoluteY();
    inline byte readAddrZeroPageY();
    inline byte readAddrXIndirect();
    inline byte readAddrIndirectY();

    //write instructions which return the address in memory to write to for a given addressing mode.
    inline byte writeAddrZeroPage();
    inline byte writeAddrZeroPageX();
    inline byte writeAddrZeroPageY();
    inline word writeAddrAbsolute();
    inline word writeAddrAbsoluteX();
    inline word writeAddrAbsoluteY();
    inline word writeAddrXIndirect();
    inline word writeAddrIndirectY();

    void pushByteToStack(byte data);
    void pushWordToStack(word data);
    word SPToAddress(bool incrementSP=false);
    byte pullByteFromStack(bool incSPBefore = false, bool incSPAfter = false);
};

#endif //INC_6502_EMULATION_6502_H