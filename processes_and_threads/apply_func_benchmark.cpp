#include "apply_func.h"

#include <benchmark/benchmark.h>
#include <cmath>
#include <vector>

static void LightTransform(int& x) {
    x += 1;
}

static void HeavyTransform(int& x) {
    double value = static_cast<double>(x);

    for (int i = 0; i < 200; ++i) {
        value = std::sin(value) + std::cos(value) + std::sqrt(std::abs(value) + 1.0);
    }

    x = static_cast<int>(value);
}

static std::vector<int> MakeVector(std::size_t size) {
    std::vector<int> data(size);
    for (std::size_t i = 0; i < size; ++i) {
        data[i] = static_cast<int>(i);
    }
    return data;
}

static void BM_SmallVector_OneThread(benchmark::State& state) {
    for (auto _ : state) {
        auto data = MakeVector(1000);
        ApplyFunction<int>(data, LightTransform, 1);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_SmallVector_OneThread);

static void BM_SmallVector_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        auto data = MakeVector(1000);
        ApplyFunction<int>(data, LightTransform, 4);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_SmallVector_MultiThread);

static void BM_BigVector_OneThread(benchmark::State& state) {
    for (auto _ : state) {
        auto data = MakeVector(200000);
        ApplyFunction<int>(data, HeavyTransform, 1);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_BigVector_OneThread);

static void BM_BigVector_MultiThread(benchmark::State& state) {
    for (auto _ : state) {
        auto data = MakeVector(200000);
        ApplyFunction<int>(data, HeavyTransform, 4);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_BigVector_MultiThread);

BENCHMARK_MAIN();