#include <benchmark/benchmark.h>
#include <cmath>
#include <vector>
#include "apply_function.h"

static void BM_Light_Single(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> data(500, 1);
        state.ResumeTiming();
        ApplyFunction<int>(data, [](int& x) { x += 1; }, 1);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Light_Single);

static void BM_Light_Multi(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> data(500, 1);
        state.ResumeTiming();
        ApplyFunction<int>(data, [](int& x) { x += 1; }, 8);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Light_Multi);

static void BM_Heavy_Single(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<double> data(100'000, 1.1);
        state.ResumeTiming();
        ApplyFunction<double>(data, [](double& x) {
            for(int i=0; i<50; ++i) x = std::sqrt(std::sin(x) * std::cos(x) + 2.0);
        }, 1);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Heavy_Single);

static void BM_Heavy_Multi(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<double> data(100'000, 1.1);
        state.ResumeTiming();
        ApplyFunction<double>(data, [](double& x) {
            for(int i=0; i<50; ++i) x = std::sqrt(std::sin(x) * std::cos(x) + 2.0);
        }, 8);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Heavy_Multi);

BENCHMARK_MAIN();
