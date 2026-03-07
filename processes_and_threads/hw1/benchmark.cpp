#include <benchmark/benchmark.h>
#include "applyfunction.h"
#include <cmath>
#include <vector>
#include <thread>
#include <atomic>

const int DATA_SIZES[] = {100, 1000, 10000, 100000, 1000000};
const int THREAD_COUNTS[] = {1, 2, 4, 8};

static void BM_SUM(benchmark::State& state) {
    int size = state.range(0);
    int threads = state.range(1);
    std::vector<int> data(size, 1);
    
    for (auto _ : state) {
        ApplyFunction<int>(data, [](int& x) { x += 1; }, threads);
    }
}


BENCHMARK(BM_SUM)
    ->Args({100, 1})
    ->Args({100, 2})
    ->Args({100, 4})
    ->Args({100, 8})
    ->Args({1000, 1})
    ->Args({1000, 2})
    ->Args({1000, 4})
    ->Args({1000, 8})
    ->Args({10000, 1})
    ->Args({10000, 2})
    ->Args({10000, 4})
    ->Args({10000, 8})
    ->Args({100000, 1})
    ->Args({100000, 2})
    ->Args({100000, 4})
    ->Args({100000, 8})
    ->Args({1000000, 1})
    ->Args({1000000, 2})
    ->Args({1000000, 4})
    ->Args({1000000, 8})
    ->Unit(benchmark::kMicrosecond);

static void BM_EXAMPLE(benchmark::State& state) {
    int size = state.range(0);
    int threads = state.range(1);
    std::vector<double> data(size, 1.5);
    
    for (auto _ : state) {
        ApplyFunction<double>(data, [](double& x) { 
            x = x * 2.5 + 1.0 - x / 3.0;
        }, threads);
    }
}

BENCHMARK(BM_EXAMPLE)
    ->Args({100, 1})->Args({100, 2})->Args({100, 4})->Args({100, 8})
    ->Args({1000, 1})->Args({1000, 2})->Args({1000, 4})->Args({1000, 8})
    ->Args({10000, 1})->Args({10000, 2})->Args({10000, 4})->Args({10000, 8})
    ->Args({100000, 1})->Args({100000, 2})->Args({100000, 4})->Args({100000, 8})
    ->Args({1000000, 1})->Args({1000000, 2})->Args({1000000, 4})->Args({1000000, 8})
    ->Unit(benchmark::kMicrosecond);


static void BM_SINCOS(benchmark::State& state) {
    int size = state.range(0);
    int threads = state.range(1);
    std::vector<double> data(size, 1.5);
    
    for (auto _ : state) {
        ApplyFunction<double>(data, [](double& x) { 
            for (int i = 0; i < 10; i++) {
                x = sin(x) * cos(x) + tan(x/2) - exp(sin(x));
            }
        }, threads);
    }
}

BENCHMARK(BM_SINCOS)
    ->Args({100, 1})->Args({100, 2})->Args({100, 4})->Args({100, 8})
    ->Args({1000, 1})->Args({1000, 2})->Args({1000, 4})->Args({1000, 8})
    ->Args({10000, 1})->Args({10000, 2})->Args({10000, 4})->Args({10000, 8})
    ->Args({100000, 1})->Args({100000, 2})->Args({100000, 4})->Args({100000, 8})
    ->Args({1000000, 1})->Args({1000000, 2})->Args({1000000, 4})->Args({1000000, 8})
    ->Unit(benchmark::kMicrosecond);


static void BM_NUMThreads(benchmark::State& state) {
    int threads = state.range(0);
    int size = 100000;
    std::vector<double> data(size, 1.5);
    
    for (auto _ : state) {
        ApplyFunction<double>(data, [](double& x) {
            for (int i = 0; i < 5; i++) {
                x = sin(x) * cos(x);
            }
        }, threads);
    }
}

BENCHMARK(BM_NUMThreads)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16)
    ->Unit(benchmark::kMicrosecond);


static void BM_DATA(benchmark::State& state) {
    int size = state.range(0);
    int threads = 4;
    std::vector<double> data(size, 1.5);
    
    for (auto _ : state) {
        ApplyFunction<double>(data, [](double& x) {
            x = sin(x) * cos(x);
        }, threads);
    }
}

BENCHMARK(BM_DATA)
    ->Arg(100)->Arg(1000)->Arg(10000)->Arg(100000)->Arg(1000000)
    ->Unit(benchmark::kMicrosecond);


BENCHMARK_MAIN();