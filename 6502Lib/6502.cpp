#include "6502.h"

using namespace m6502;

//return the number of cycles that were used
dword CPU::execute(uint64_t instructionsToExecute) {
    cycles.reset();

    auto loadRegister = [this](byte value, byte& Register) {
        Register = value;
        loadRegisterSetStatus(Register);
    };
    while(instructionsToExecute--) {
        byte instruction{fetchByte()};
        switch (instruction) {
            case INS_LDA_IM : /*2 cycles*/ {
                loadRegister(fetchByte(), A);
            } break;
            case INS_LDX_IM : {
                loadRegister(fetchByte(), X);
            } break;
            case INS_LDY_IM : {
                loadRegister(fetchByte(), Y);
            } break;
            case INS_LDA_ZP : /*3 cycles*/ {
                loadRegister(readAddrZeroPage(), A);
            } break;
            case INS_LDX_ZP : /*3 cycles*/ {
                loadRegister(readAddrZeroPage(), X);
            } break;
            case INS_LDY_ZP : /*3 cycles*/ {
                loadRegister(readAddrZeroPage(), Y);
            } break;
            case INS_LDA_ZPX : /*4 cycles*/ {
                loadRegister(readAddrZeroPageX(), A);
            } break;
            case INS_LDY_ZPX : /*4 cycles*/ {
                loadRegister(readAddrZeroPageX(), Y);
            } break;
            case INS_LDX_ZPY : {
                loadRegister(readAddrZeroPageY(), X);
            } break;
            case INS_LDA_ABS : {
                loadRegister(readAddrAbsolute(), A);
            } break;
            case INS_LDX_ABS : {
                loadRegister(readAddrAbsolute(), X);
            } break;
            case INS_LDY_ABS : {
                loadRegister(readAddrAbsolute(), Y);
            } break;
            case INS_LDA_ABSX : {
                loadRegister(readAddrAbsoluteX(), A);
            } break;
            case INS_LDY_ABSX : {
                loadRegister(readAddrAbsoluteX(), Y);
            } break;
            case INS_LDA_ABSY : {
                loadRegister(readAddrAbsoluteY(), A);
            } break;
            case INS_LDX_ABSY : {
                loadRegister(readAddrAbsoluteY(), X);
            } break;
            case INS_LDA_XIND : {
                loadRegister(readAddrXIndirect(), A);
            } break;
            case INS_LDA_INDY : {
                loadRegister(readAddrIndirectY(), A);
            } break;
            case INS_STA_ZP : {
                writeByte(A, writeAddrZeroPage());
            } break;
            case INS_STX_ZP : {
                writeByte(X, writeAddrZeroPage());
            } break;
            case INS_STY_ZP : {
                writeByte(Y, writeAddrZeroPage());
            } break;
            case INS_STA_ZPX : {
                writeByte(A, writeAddrZeroPageX());
            } break;
            case INS_STX_ZPY : {
                writeByte(X, writeAddrZeroPageY());
            } break;
            case INS_STY_ZPX : {
                writeByte(Y, writeAddrZeroPageX());
            } break;
            case INS_STA_ABS : {
                writeByte(A, writeAddrAbsolute());
            } break;
            case INS_STX_ABS : {
                writeByte(X, writeAddrAbsolute());
            } break;
            case INS_STY_ABS : {
                writeByte(Y, writeAddrAbsolute());
            } break;
            case INS_STA_ABSX : {
                writeByte(A, writeAddrAbsoluteX());
            } break;
            case INS_STA_ABSY : {
                writeByte(A, writeAddrAbsoluteY());
            } break;
            case INS_STA_XIND: {
                writeByte(A, writeAddrXIndirect());
            } break;
            case INS_STA_INDY : {
                writeByte(A, writeAddrIndirectY());
            } break;
            case INS_JSR: /*6 cycles*/ {
                byte subAddrLow = fetchByte();
                ++cycles;   //internal operation
                pushWordToStack(PC);
                PC = (fetchByte() << 8) | subAddrLow;
            } break;
            case INS_RTS : {
                readByte(PC);
                byte PCL = pullByteFromStack(true, true);
                byte PCH = pullByteFromStack();
                PC = (PCH << 8) | PCL;
                PC++;
                ++cycles;
            } break;
            case INS_JMP_ABS : {
                PC = fetchWord();
            } break;
            case INS_JMP_IND : {
                word pointer{fetchWord()};
                byte latch{readByte(pointer)};
                PC = (readByte((pointer & 0x00FF) == 0xFF ? (pointer & 0xFF00) : pointer + 1) << 8) | latch;
            } break;
            case INS_PHA_IMP : {
                fetchByte();
                pushByteToStack(A);
            } break;
            case INS_PHP_IMP : {
                fetchByte();
                pushByteToStack(static_cast<byte>(PS.to_ulong()));
            } break;
            case INS_PLA_IMP : {
                fetchByte();
                loadRegister(pullByteFromStack(true), A);
            } break;
            case INS_PLP_IMP : {
                fetchByte();
                PS = pullByteFromStack(true);
            } break;
            case INS_TSX_IMP : {
                loadRegister(SP, X);
                fetchByte();
            } break;
            case INS_TXS_IMP : {
                SP = X;
                fetchByte();
            } break;
            default: {
                std::cerr << "Error: instruction " << instruction << "not handled" << std::endl;
                goto INSTRUCTION_NOT_HANDLED;
            }
        }
    }
    INSTRUCTION_NOT_HANDLED:
    return cycles.getCycles();
}

void CPU::reset() {
    PC = mem[0xFFFC] | (mem[0xFFFD] << 8);
    SP = 0xFF;
    PS.reset();
    A = X = Y = 0;
}

byte CPU::readByte(word address) {
    CyclesIncrementer cd(cycles);
    return mem[address];
}

word CPU::readWord(word address) {
    word data = readByte(address);
    return data | (readByte(address + 1) << 8);
}

byte CPU::fetchByte() {
    CyclesIncrementer cd(cycles);
    return mem[PC++];
}

/*    6502 is little Endian which means that the first byte read
    * will be the least significant byte. That is if you want to
    * store 0x1234 in memory then it will be stored as 34 12. What
    * the code below for our example will first read 34 from memory
    * as a word so it will be 0034. Then it reads the next byte from
    * memory which is 12 shifts it to the left by 8 bits = 1200 and
    * then does 0034 | 1200 = 1234 which is what we wanted.*/
word CPU::fetchWord() {
    word data = fetchByte();
    return data | (fetchByte() << 8);
}

/*
     * 0x1234 will be stored as 34 12 so write LSB bytes at low address
     * and MSB bytes at high address.
     */
void CPU::writeWord(word data, word address) {
    writeByte(data & 0xFF, address);
    writeByte(data >> 8, address + 1);
}

void CPU::writeByte(byte data, word address) {
    mem[address] = data;
    ++cycles;
}

void CPU::loadRegisterSetStatus(byte Register) {
    PS.set(StatusFlags::Z, Register == 0);
    PS.set(StatusFlags::N , (Register & 0b10000000) > 0);
}

void CPU::pushByteToStack(byte data) {
    writeByte(data, SPToAddress());
    SP--;
}

void CPU::pushWordToStack(word data) {
    pushByteToStack((data & 0xFF00) >> 8);
    pushByteToStack(data);
}

byte CPU::pullByteFromStack(bool incSPBefore, bool incSPAfter) {
    if(incSPBefore) {
        SP++;
        ++cycles;
    }
    return readByte(SPToAddress(incSPAfter));
}

//return the SP as a full 16 bit address in the first page even though SP is a byte
word CPU::SPToAddress(bool incrementSP) {
    return incrementSP ? 0x100 | SP++ : 0x100 | SP;
}

byte CPU::readAddrZeroPage() {
    byte address{fetchByte()};
    return readByte(address);
}

byte CPU::writeAddrZeroPage() {
    return fetchByte();
}

byte CPU::readAddrZeroPageX() {
    byte address{fetchByte()};
    byte effectiveAddress = address + X;
    ++cycles;
    return readByte(effectiveAddress);
}

byte CPU::writeAddrZeroPageX() {
    ++cycles;
    return fetchByte() + X;
}

byte CPU::readAddrZeroPageY() {
    byte address{fetchByte()};
    byte effectiveAddress = address + Y;
    ++cycles;
    return readByte(effectiveAddress);
}

byte CPU::writeAddrZeroPageY() {
    ++cycles;
    return fetchByte() + Y;
}

byte CPU::readAddrAbsolute() {
    word address{fetchWord()};
    return readByte(address);
}

word CPU::writeAddrAbsolute() {
    return fetchWord();
}

byte CPU::readAddrAbsoluteX() {
    word address = fetchWord();
    dword effectiveAddress = address + X;
    byte data{readByte(effectiveAddress)};
    return (((address & 0xFF) + X) > 0xFF) ? readByte(effectiveAddress - 0x100) : data;
}

word CPU::writeAddrAbsoluteX() {
    word address = fetchWord();
    dword effectiveAddress = address + X;
    ++cycles;
    return (((address & 0xFF) + X) > 0xFF) ? effectiveAddress - 0x100 : effectiveAddress;
}

byte CPU::readAddrAbsoluteY() {
    word address = fetchWord();
    dword effectiveAddress = address + Y;
    byte data{readByte(effectiveAddress)};
    return (((address & 0xFF) + Y) > 0xFF) ? readByte(effectiveAddress - 0x100) : data;
}

word CPU::writeAddrAbsoluteY() {
    word address = fetchWord();
    dword effectiveAddress = address + Y;
    ++cycles;
    return (((address & 0xFF) + Y) > 0xFF) ? effectiveAddress - 0x100 : effectiveAddress;
}

byte CPU::readAddrXIndirect() {
    byte startAddress = (fetchByte() + X) & 0xFF;
    ++cycles;
    word effectiveAddress = readByte(startAddress) | (readByte((startAddress + 0x01) & 0xFF)) << 8;
    return readByte(effectiveAddress);
}

word CPU::writeAddrXIndirect() {
    byte startAddress = (fetchByte() + X) & 0xFF;
    ++cycles;
    return readByte(startAddress) | (readByte((startAddress + 0x01) & 0xFF)) << 8;
}

byte CPU::readAddrIndirectY() {
    byte zpAddress = fetchByte();
    word address = readWord(zpAddress);
    cycles += (((address & 0xFF) + Y) > 0xFF);
    word effectiveAddress = address + Y;
    return readByte(effectiveAddress);
}

word CPU::writeAddrIndirectY() {
    byte zpAddress = fetchByte();
    word address = readWord(zpAddress);
    ++cycles;
    return address + Y;
}
