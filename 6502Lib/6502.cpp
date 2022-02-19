#include "6502.h"

using namespace m6502;

//return the number of cycles that were used
dword CPU::execute(dword unsignedCycles, dword instructionsToExecute) {
    const dword cyclesPassedIn = unsignedCycles;
    Cycles cycles(static_cast<sdword>(unsignedCycles), this->TCSFrequency, 1);

    auto loadRegister = [this](byte value, byte& Register) {
        Register = value;
        loadRegisterSetStatus(Register);
    };
    while(cycles > 0 && instructionsToExecute) {
        byte instruction{fetchByte(cycles)};
        instructionsToExecute--;
        switch (instruction) {
            case INS_LDA_IM : /*2 cycles*/ {
                loadRegister(fetchByte(cycles), A);
            } break;
            case INS_LDX_IM : {
                loadRegister(fetchByte(cycles), X);
            } break;
            case INS_LDY_IM : {
                loadRegister(fetchByte(cycles), Y);
            } break;
            case INS_LDA_ZP : /*3 cycles*/ {
                loadRegister(readAddrZeroPage(cycles), A);
            } break;
            case INS_LDX_ZP : /*3 cycles*/ {
                loadRegister(readAddrZeroPage(cycles), X);
            } break;
            case INS_LDY_ZP : /*3 cycles*/ {
                loadRegister(readAddrZeroPage(cycles), Y);
            } break;
            case INS_LDA_ZPX : /*4 cycles*/ {
                loadRegister(readAddrZeroPageX(cycles), A);
            } break;
            case INS_LDY_ZPX : /*4 cycles*/ {
                loadRegister(readAddrZeroPageX(cycles), Y);
            } break;
            case INS_LDX_ZPY : {
                loadRegister(readAddrZeroPageY(cycles), X);
            } break;
            case INS_LDA_ABS : {
                loadRegister(readAddrAbsolute(cycles), A);
            } break;
            case INS_LDX_ABS : {
                loadRegister(readAddrAbsolute(cycles), X);
            } break;
            case INS_LDY_ABS : {
                loadRegister(readAddrAbsolute(cycles), Y);
            } break;
            case INS_LDA_ABSX : {
                loadRegister(readAddrAbsoluteX(cycles), A);
            } break;
            case INS_LDY_ABSX : {
                loadRegister(readAddrAbsoluteX(cycles), Y);
            } break;
            case INS_LDA_ABSY : {
                loadRegister(readAddrAbsoluteY(cycles), A);
            } break;
            case INS_LDX_ABSY : {
                loadRegister(readAddrAbsoluteY(cycles), X);
            } break;
            case INS_LDA_XIND : {
                loadRegister(readAddrXIndirect(cycles), A);
            } break;
            case INS_LDA_INDY : {
                loadRegister(readAddrIndirectY(cycles), A);
            } break;
            case INS_STA_ZP : {
                writeByte(A, writeAddrZeroPage(cycles), cycles);
            } break;
            case INS_STX_ZP : {
                writeByte(X, writeAddrZeroPage(cycles), cycles);
            } break;
            case INS_STY_ZP : {
                writeByte(Y, writeAddrZeroPage(cycles), cycles);
            } break;
            case INS_STA_ZPX : {
                writeByte(A, writeAddrZeroPageX(cycles), cycles);
            } break;
            case INS_STX_ZPY : {
                writeByte(X, writeAddrZeroPageY(cycles), cycles);
            } break;
            case INS_STY_ZPX : {
                writeByte(Y, writeAddrZeroPageX(cycles), cycles);
            } break;
            case INS_STA_ABS : {
                writeByte(A, writeAddrAbsolute(cycles), cycles);
            } break;
            case INS_STX_ABS : {
                writeByte(X, writeAddrAbsolute(cycles), cycles);
            } break;
            case INS_STY_ABS : {
                writeByte(Y, writeAddrAbsolute(cycles), cycles);
            } break;
            case INS_STA_ABSX : {
                writeByte(A, writeAddrAbsoluteX(cycles), cycles);
            } break;
            case INS_STA_ABSY : {
                writeByte(A, writeAddrAbsoluteY(cycles), cycles);
            } break;
            case INS_STA_XIND: {
                writeByte(A, writeAddrXIndirect(cycles), cycles);
            } break;
            case INS_STA_INDY : {
                writeByte(A, writeAddrIndirectY(cycles), cycles);
            } break;
            case INS_JSR: /*6 cycles*/ {
                byte subAddrLow = fetchByte(cycles);
                --cycles;   //internal operation
                pushWordToStack(PC, cycles);
                PC = (fetchByte(cycles) << 8) | subAddrLow;
            } break;
            case INS_RTS : {
                readByte(PC, cycles);
                byte PCL = pullByteFromStack(cycles, true, true);
                byte PCH = pullByteFromStack(cycles);
                PC = (PCH << 8) | PCL;
                PC++;
                --cycles;
            } break;
            default: {
                std::cerr << "Error: instruction " << instruction << "not handled" << std::endl;
                goto INSTRUCTION_NOT_HANDLED;
            }
        }
    }
    INSTRUCTION_NOT_HANDLED:
    return cyclesPassedIn - cycles.getCycles();
}


void CPU::reset(word address) {
    PC = address;
    SP = 0xFF;
    ps.C = ps.Z = ps.I = ps.D = ps.B = ps.V = ps.N = 0;
    A = X = Y = 0;
}

byte CPU::readByte(word address, Cycles &cycles) {
    CyclesDecrementer cd(cycles);
    return mem[address];
}

word CPU::readWord(word address, Cycles &cycles) {
    word data = readByte(address, cycles);
    return data | (readByte(address + 1, cycles) << 8);
}

byte CPU::fetchByte(Cycles &cycles) {
    CyclesDecrementer cd(cycles);
    return mem[PC++];
}

/*    6502 is little Endian which means that the first byte read
    * will be the least significant byte. That is if you want to
    * store 0x1234 in memory then it will be stored as 34 12. What
    * the code below for our example will first read 34 from memory
    * as a word so it will be 0034. Then it reads the next byte from
    * memory which is 12 shifts it to the left by 8 bits = 1200 and
    * then does 0034 | 1200 = 1234 which is what we wanted.*/
word CPU::fetchWord(Cycles &cycles) {
    word data = fetchByte(cycles);
    return data | (fetchByte(cycles) << 8);
}

/*
     * 0x1234 will be stored as 34 12 so write LSB bytes at low address
     * and MSB bytes at high address.
     */
void CPU::writeWord(word data, word address, Cycles &cycles) {
    writeByte(data & 0xFF, address, cycles);
    writeByte(data >> 8, address + 1, cycles);
}

void CPU::writeByte(byte data, word address, Cycles &cycles) {
    mem[address] = data;
    --cycles;
}

void CPU::loadRegisterSetStatus(byte Register) {
    ps.Z = (Register == 0);
    ps.N = (Register & 0b10000000) > 0;
}

void CPU::pushByteToStack(byte data, Cycles &cycles) {
    writeByte(data, SPToAddress(), cycles);
    SP--;
}

void CPU::pushWordToStack(word data, Cycles &cycles) {
    pushByteToStack((data & 0xFF00) >> 8, cycles);
    pushByteToStack(data, cycles);
}

byte CPU::pullByteFromStack(Cycles &cycles, bool incSPBefore, bool incSPAfter) {
    if(incSPBefore) {
        SP++;
        --cycles;
    }
    return readByte(SPToAddress(incSPAfter), cycles);
}

//return the SP as a full 16 bit address in the first page even though SP is a byte
word CPU::SPToAddress(bool incrementSP) {
    return incrementSP ? 0x100 | SP++ : 0x100 | SP;
}

byte CPU::readAddrZeroPage(Cycles &cycles) {
    byte address{fetchByte(cycles)};
    return readByte(address, cycles);
}

byte CPU::writeAddrZeroPage(Cycles &cycles) {
    return fetchByte(cycles);
}

byte CPU::readAddrZeroPageX(Cycles &cycles) {
    byte address{fetchByte(cycles)};
    byte effectiveAddress = address + X;
    return readByte(effectiveAddress, --cycles);
}

byte CPU::writeAddrZeroPageX(Cycles &cycles) {
    return fetchByte(--cycles) + X;
}

byte CPU::readAddrZeroPageY(Cycles &cycles) {
    byte address{fetchByte(cycles)};
    byte effectiveAddress = address + Y;
    return readByte(effectiveAddress, --cycles);
}

byte CPU::writeAddrZeroPageY(Cycles &cycles) {
    return fetchByte(--cycles) + Y;
}

byte CPU::readAddrAbsolute(Cycles &cycles) {
    word address{fetchWord(cycles)};
    return readByte(address, cycles);
}

word CPU::writeAddrAbsolute(Cycles &cycles) {
    return fetchWord(cycles);
}

byte CPU::readAddrAbsoluteX(Cycles &cycles) {
    word address = fetchWord(cycles);
    dword effectiveAddress = address + X;
    byte data{readByte(effectiveAddress, cycles)};
    return (((address & 0xFF) + X) > 0xFF) ? readByte(effectiveAddress - 0x100, cycles) : data;
}

word CPU::writeAddrAbsoluteX(Cycles &cycles) {
    word address = fetchWord(cycles);
    dword effectiveAddress = address + X;
    --cycles;
    return (((address & 0xFF) + X) > 0xFF) ? effectiveAddress - 0x100 : effectiveAddress;
}

byte CPU::readAddrAbsoluteY(Cycles &cycles) {
    word address = fetchWord(cycles);
    dword effectiveAddress = address + Y;
    byte data{readByte(effectiveAddress, cycles)};
    return (((address & 0xFF) + Y) > 0xFF) ? readByte(effectiveAddress - 0x100, cycles) : data;
}

word CPU::writeAddrAbsoluteY(Cycles &cycles) {
    word address = fetchWord(cycles);
    dword effectiveAddress = address + Y;
    --cycles;
    return (((address & 0xFF) + Y) > 0xFF) ? effectiveAddress - 0x100 : effectiveAddress;
}

byte CPU::readAddrXIndirect(Cycles &cycles) {
    byte startAddress = (fetchByte(cycles) + X) & 0xFF;
    word effectiveAddress = readByte(startAddress, --cycles) | (readByte((startAddress + 0x01) & 0xFF, cycles)) << 8;
    return readByte(effectiveAddress, cycles);
}

word CPU::writeAddrXIndirect(Cycles &cycles) {
    byte startAddress = (fetchByte(cycles) + X) & 0xFF;
    return readByte(startAddress, --cycles) | (readByte((startAddress + 0x01) & 0xFF, cycles)) << 8;
}

byte CPU::readAddrIndirectY(Cycles &cycles) {
    byte zpAddress = fetchByte(cycles);
    word address = readWord(zpAddress, cycles);
    cycles -= (((address & 0xFF) + Y) > 0xFF);
    word effectiveAddress = address + Y;
    return readByte(effectiveAddress, cycles);
}

word CPU::writeAddrIndirectY(Cycles &cycles) {
    byte zpAddress = fetchByte(cycles);
    word address = readWord(zpAddress, cycles);
    --cycles;
    return address + Y;
}
