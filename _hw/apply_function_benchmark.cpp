#include <benchmark/benchmark.h>
#include "apply_function.h"
#include <cmath>
#include <numeric>

static void BM_LightTransform(benchmark::State& state) {
    const int size = state.range(0);
    const int threads = state.range(1);
    std::vector<int> data(size);

    std::function<void(int&)> light = [](int& x) { x += 1; };

    for (auto _ : state) {
        std::iota(data.begin(), data.end(), 0);
        ApplyFunction<int>(data, light, threads);
        benchmark::DoNotOptimize(data.data());
    }
}

BENCHMARK(BM_LightTransform)
    ->Args({100, 1})
    ->Args({100, 4})
    ->Args({100, 8})
    ->Args({1000, 1})
    ->Args({1000, 4})
    ->Args({1000, 8})
    ->Unit(benchmark::kMicrosecond);

static void BM_HeavyTransform(benchmark::State& state) {
    const int size = state.range(0);
    const int threads = state.range(1);
    std::vector<double> data(size);

    std::function<void(double&)> heavy = [](double& x) {
        for (int i = 0; i < 2000; ++i)
            x = std::sin(x) + std::cos(x) + std::sqrt(std::abs(x) + 1.0);
    };

    for (auto _ : state) {
        std::iota(data.begin(), data.end(), 0);
        ApplyFunction<double>(data, heavy, threads);
        benchmark::DoNotOptimize(data.data());
    }
}

BENCHMARK(BM_HeavyTransform)
    ->Args({10000, 1})
    ->Args({10000, 2})
    ->Args({10000, 4})
    ->Args({10000, 8})
    ->Args({100000, 1})
    ->Args({100000, 4})
    ->Args({100000, 8})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
