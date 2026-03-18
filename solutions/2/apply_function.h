#pragma once

#include <functional>
#include <thread>
#include <vector>

template <typename T>
void ApplyFunction(std::vector<T>& data, const std::function<void(T&)>& transform,
                   const int threadCount = 1) {
    const int n = static_cast<int>(data.size());
    if (n == 0) {
        return;
    }

    const int threads = std::min(threadCount, n);

    if (threads <= 1) {
        for (auto& elem : data) {
            transform(elem);
        }
        return;
    }

    std::vector<std::thread> pool;
    pool.reserve(threads);

    const int chunkSize = n / threads;
    const int remainder = n % threads;

    int start = 0;
    for (int i = 0; i < threads; ++i) {
        const int end = start + chunkSize + (i < remainder ? 1 : 0);
        pool.emplace_back([&data, &transform, start, end]() {
            for (int j = start; j < end; ++j) {
                transform(data[j]);
            }
        });
        start = end;
    }

    for (auto& t : pool) {
        t.join();
    }
}
