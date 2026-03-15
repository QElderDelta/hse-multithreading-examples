#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <algorithm>

template <typename T>
void ApplyFunction(std::vector<T>& data, const std::function<void(T&)>& transform, const int threadCount = 1) {
    if (data.empty() || threadCount <= 0) return;

    const int n = static_cast<int>(data.size());
    const int actualThreads = std::min(threadCount, n);

    if (actualThreads <= 1) {
        for (auto& el : data) transform(el);
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(actualThreads);

    const int chunkSize = n / actualThreads;
    const int remainder = n % actualThreads;

    int offset = 0;
    for (int i = 0; i < actualThreads; ++i) {
        int count = chunkSize + (i < remainder ? 1 : 0);
        threads.emplace_back([&data, &transform, offset, count]() {
            for (int j = offset; j < offset + count; ++j) {
                transform(data[j]);
            }
        });
        offset += count;
    }

    for (auto& t : threads) t.join();
}
