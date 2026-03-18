#include "apply_function.h"

#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include <cmath>
#include <algorithm>

static int DefaultThreads()
{
    unsigned hc = std::thread::hardware_concurrency();
    if (hc == 0) hc = 4;
    return static_cast<int>(std::max(2u, hc));
}

static void BM_SmallCheap_1Thread(benchmark::State& state)
{
    const std::size_t n = 1024;
    std::vector<int> data(n, 1);

    const std::function<void(int&)> transform = [](int& x) { x += 1; };

    for (auto _ : state) {
        ApplyFunction<int>(data, transform, 1);
        benchmark::DoNotOptimize(data.data());
    }
}

static void BM_SmallCheap_MultiThread(benchmark::State& state)
{
    const std::size_t n = 1024;
    std::vector<int> data(n, 1);

    const int threads = DefaultThreads();
    const std::function<void(int&)> transform = [](int& x) { x += 1; };

    for (auto _ : state) {
        ApplyFunction<int>(data, transform, threads);
        benchmark::DoNotOptimize(data.data());
    }
}

static void BM_LargeExpensive_1Thread(benchmark::State& state)
{
    const std::size_t n = 1'000'000;
    std::vector<double> data(n, 0.001);

    const std::function<void(double&)> transform = [](double& x) {
        double v = x;
        for (int i = 0; i < 200; ++i) {
            v = v * 1.0000001 + 0.0000001;
            v = std::sqrt(std::abs(v)) + 1.0;
        }
        x = v;
    };

    for (auto _ : state) {
        ApplyFunction<double>(data, transform, 1);
        benchmark::DoNotOptimize(data.data());
    }
}

static void BM_LargeExpensive_MultiThread(benchmark::State& state)
{
    const std::size_t n = 1'000'000;
    std::vector<double> data(n, 0.001);

    const int threads = DefaultThreads();

    const std::function<void(double&)> transform = [](double& x) {
        double v = x;
        for (int i = 0; i < 200; ++i) {
            v = v * 1.0000001 + 0.0000001;
            v = std::sqrt(std::abs(v)) + 1.0;
        }
        x = v;
    };

    for (auto _ : state) {
        ApplyFunction<double>(data, transform, threads);
        benchmark::DoNotOptimize(data.data());
    }
}

BENCHMARK(BM_SmallCheap_1Thread);
BENCHMARK(BM_SmallCheap_MultiThread);
BENCHMARK(BM_LargeExpensive_1Thread);
BENCHMARK(BM_LargeExpensive_MultiThread);

BENCHMARK_MAIN();