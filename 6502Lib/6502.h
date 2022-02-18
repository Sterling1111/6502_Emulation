#ifndef INC_6502_EMULATION_6502_H
#define INC_6502_EMULATION_6502_H

#include <iostream>

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

    struct PS {     //processor status
        byte C : 1;  //carry flag
        byte Z : 1;  //zero flag
        byte I : 1;  //interrupt disable
        byte D : 1;  //decimal mode
        byte B : 1;  //break command
        byte V : 1;  //overflow flag
        byte N : 1;  //negative flag
    };
    PS ps;
    struct Mem {
        static constexpr dword MAX_MEM = 1024 * 64;
        byte data[MAX_MEM];

        Mem() {initialize();}

        void initialize() {
            for (dword i{0}; i < MAX_MEM; i++)
                data[i] = 0;
        }

        byte operator[](dword address) const {
            return data[address];
        }

        byte& operator[](dword address) {
            return data[address];
        }
    };
    Mem mem;

    struct CyclesDecrementer {
        sdword& cycles;
        explicit CyclesDecrementer(sdword& cycles) : cycles{cycles} {};
        ~CyclesDecrementer() { --cycles; }
    };

    //opcode for the instruction LDA which is Load Accumulator
    //in Immediate addressing mode.
    static constexpr byte
    //LDA
    INS_LDA_IM = 0xA9,  INS_LDA_ZP = 0xA5, INS_LDA_ZPX = 0xB5, INS_LDA_ABS = 0xAD, INS_LDA_ABSX = 0xBD, INS_LDA_ABSY = 0xB9, INS_LDA_XIND = 0xA1, INS_LDA_INDY = 0xB1,
    //LDX
    INS_LDX_IM = 0xA2,  INS_LDX_ZP = 0xA6, INS_LDX_ZPY = 0xB6, INS_LDX_ABS = 0xAE, INS_LDX_ABSY = 0xBE,
    //LDY
    INS_LDY_IM = 0xA0,  INS_LDY_ZP = 0xA4, INS_LDY_ZPX = 0xB4, INS_LDY_ABS = 0xAC, INS_LDY_ABSX = 0xBC,
    //STA
    INS_STA_ZP = 0x85, INS_STA_ZPX = 0x95, INS_STA_ABS = 0x8D, INS_STA_ABSX = 0x9D, INS_STA_ABSY = 0x99, INS_STA_XIND = 0x81, INS_STA_INDY = 0x91,
    //STX
    INS_STX_ZP = 0x86, INS_STX_ZPY = 0x96, INS_STX_ABS = 0x8E,
    //STY
    INS_STY_ZP = 0x84, INS_STY_ZPX = 0x94, INS_STY_ABS = 0x8C,
    //JSR
    INS_JSR = 0x20,
    INS_RTS = 0x60;

    CPU(){reset();};
    void reset(word = 0xFFFC);
    word readWord(word address, sdword& cycles);
    byte readByte(word address, sdword& cycles);
    byte fetchByte(sdword& cycles);
    word fetchWord(sdword& cycles);
    void writeWord(word data, word address, sdword& cycles);
    void writeByte(byte data, word address, sdword& cycles);
    void loadRegisterSetStatus(byte Register);
    dword execute(dword, dword = 1);
    //read instructions which return the byte in memory at the address for the given addressing mode
    inline byte readAddrZeroPage(sdword &cycles);
    inline byte readAddrZeroPageX(sdword &cycles);
    inline byte readAddrAbsolute(sdword &signedCycles);
    inline byte readAddrAbsoluteX(sdword &signedCycles);
    inline byte readAddrAbsoluteY(sdword &signedCycles);
    inline byte readAddrZeroPageY(sdword &signedCycles);
    inline byte readAddrXIndirect(sdword &signedCycles);
    inline byte readAddrIndirectY(sdword &cycles);

    //write instructions which return the address in memory to write to for a given addressing mode.
    inline byte writeAddrZeroPage(sdword &cycles);
    inline byte writeAddrZeroPageX(sdword &cycles);
    inline byte writeAddrZeroPageY(sdword &cycles);
    inline word writeAddrAbsolute(sdword &cycles);
    inline word writeAddrAbsoluteX(sdword&);
    inline word writeAddrAbsoluteY(sdword &cycles);
    inline word writeAddrXIndirect(sdword &cycles);
    inline word writeAddrIndirectY(sdword &cycles);

    void pushByteToStack(byte, sdword&);
    void pushWordToStack(word, sdword&);
    word SPToAddress(bool=false);
    byte pullByteFromStack(sdword&, bool= false, bool= false);
};

#endif //INC_6502_EMULATION_6502_H

