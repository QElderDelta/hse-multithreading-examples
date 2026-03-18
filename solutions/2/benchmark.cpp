#include "apply_function.h"

#include <benchmark/benchmark.h>

#include <cmath>
#include <numeric>
#include <vector>

const int kThreads = std::thread::hardware_concurrency();

static void BM_Small_SingleThread(benchmark::State& state) {
    std::vector<int> data(64);
    for (auto _ : state) {
        ApplyFunction<int>(data, [](int& x) { x += 1; }, 1);
        benchmark::DoNotOptimize(data.data());
    }
}
BENCHMARK(BM_Small_SingleThread)->Unit(benchmark::kMicrosecond);

static void BM_Small_MultiThread(benchmark::State& state) {
    std::vector<int> data(64);
    for (auto _ : state) {
        ApplyFunction<int>(data, [](int& x) { x += 1; }, kThreads);
        benchmark::DoNotOptimize(data.data());
    }
}
BENCHMARK(BM_Small_MultiThread)->Unit(benchmark::kMicrosecond);

static void BM_Large_SingleThread(benchmark::State& state) {
    std::vector<double> data(1 << 20);
    std::iota(data.begin(), data.end(), 1.0);
    for (auto _ : state) {
        ApplyFunction<double>(data, [](double& x) { x = std::sqrt(x) + std::log(x); }, 1);
        benchmark::DoNotOptimize(data.data());
    }
}
BENCHMARK(BM_Large_SingleThread)->Unit(benchmark::kMillisecond);

static void BM_Large_MultiThread(benchmark::State& state) {
    std::vector<double> data(1 << 20);
    std::iota(data.begin(), data.end(), 1.0);
    for (auto _ : state) {
        ApplyFunction<double>(data, [](double& x) { x = std::sqrt(x) + std::log(x); }, kThreads);
        benchmark::DoNotOptimize(data.data());
    }
}
BENCHMARK(BM_Large_MultiThread)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
