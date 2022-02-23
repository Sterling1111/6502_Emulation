#include "benchmark/benchmark.h"
#include "6502.h"

class _6502LoadRegisterBenchmarks : public benchmark::Fixture {
public:
    const static benchmark::TimeUnit TimeUnit = benchmark::TimeUnit::kMicrosecond;
    m6502::CPU cpu{1};
    _6502LoadRegisterBenchmarks() {/* Iterations(3);*/}
    void SetUp(const ::benchmark::State& state) { cpu.PC = 0xFFFC; }
    void TearDown(const ::benchmark::State& state) {}

    m6502::byte BenchmarkLoadRegisterImmediate(m6502::byte, m6502::byte m6502::CPU::*);
    m6502::byte BenchmarkLoadRegisterZeroPage(m6502::byte, m6502::byte m6502::CPU::*);
};

m6502::byte _6502LoadRegisterBenchmarks::BenchmarkLoadRegisterImmediate(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.execute();
    cpu.PC = 0xFFFC;
    return cpu.*Register;
}

m6502::byte _6502LoadRegisterBenchmarks::BenchmarkLoadRegisterZeroPage(m6502::byte opcode, m6502::byte m6502::CPU::* Register) {
    cpu.mem[0xFFFC] = opcode;
    cpu.mem[0xFFFD] = 0x42;
    cpu.mem[0x0042] = 0x37;
    cpu.execute();
    cpu.PC = 0xFFFC;
    return cpu.*Register;
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDAImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(m6502::CPU::INS_LDA_IM, &m6502::CPU::A));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDXImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(m6502::CPU::INS_LDX_IM, &m6502::CPU::X));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDYImmediate)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterImmediate(m6502::CPU::INS_LDY_IM, &m6502::CPU::Y));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDAZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterZeroPage(m6502::CPU::INS_LDA_ZP, &m6502::CPU::A));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDXZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterZeroPage(m6502::CPU::INS_LDX_ZP, &m6502::CPU::X));
}

BENCHMARK_DEFINE_F(_6502LoadRegisterBenchmarks, LDYZeroPage)(benchmark::State& st) {
    for(auto _ : st)
        benchmark::DoNotOptimize(BenchmarkLoadRegisterZeroPage(m6502::CPU::INS_LDY_ZP, &m6502::CPU::Y));
}

BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDAImmediate)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDXImmediate)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDYImmediate)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDAZeroPage)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDXZeroPage)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();
BENCHMARK_REGISTER_F(_6502LoadRegisterBenchmarks, LDYZeroPage)->Unit(_6502LoadRegisterBenchmarks::TimeUnit)->UseRealTime();