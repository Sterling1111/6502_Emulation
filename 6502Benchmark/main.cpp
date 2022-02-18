#include "benchmark/benchmark.h"
#include "6502.h"

using namespace m6502;

class _6502LoadRegisterBenchmarks : public benchmark::Fixture {
public:
    CPU cpu;
    void SetUp(const ::benchmark::State& state) { cpu.reset(); }
    void TearDown(const ::benchmark::State& state) {}

    byte BenchmarkLoadRegisterImmediate(byte, byte CPU::*);
    byte BenchmarkLoadRegisterZeroPage(byte, byte CPU::*);
};

byte _6502LoadRegisterBenchmarks::BenchmarkLoadRegisterImmediate(byte opcode, byte CPU::* Register) {
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    constexpr int REQUIRED_CYCLES = 2;
    cpu.execute(REQUIRED_CYCLES);
    cpu.PC = 0xFFFC;
    return cpu.*Register;
}

byte _6502LoadRegisterBenchmarks::BenchmarkLoadRegisterZeroPage(byte opcode, byte CPU::* Register) {
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0042] = 0x37;
    constexpr int REQUIRED_CYCLES = 3;
    cpu.execute(REQUIRED_CYCLES);
    cpu.PC = 0xFFFC;
    return cpu.*Register;
}


BENCHMARK_F(_6502LoadRegisterBenchmarks, LDAImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(CPU::INS_LDA_IM, &CPU::A));
}

BENCHMARK_F(_6502LoadRegisterBenchmarks, LDXImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(CPU::INS_LDX_IM, &CPU::X));
}

BENCHMARK_F(_6502LoadRegisterBenchmarks, LDYImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(CPU::INS_LDY_IM, &CPU::Y));
}

BENCHMARK_F(_6502LoadRegisterBenchmarks, LDAZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(CPU::INS_LDA_ZP, &CPU::A));
}

BENCHMARK_F(_6502LoadRegisterBenchmarks, LDXZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(CPU::INS_LDX_ZP, &CPU::X));
}

BENCHMARK_F(_6502LoadRegisterBenchmarks, LDYZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(CPU::INS_LDY_ZP, &CPU::Y));
}

