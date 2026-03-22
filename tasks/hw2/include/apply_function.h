#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <exception>
#include <algorithm>

template <typename T>
void ApplyFunction(std::vector<T>& data,
                   const std::function<void(T&)>& transform,
                   const int threadCount = 1)
{
    const std::size_t n = data.size();
    if (n == 0) return;

    int tc = (threadCount <= 1) ? 1 : threadCount;
    if (static_cast<std::size_t>(tc) > n) tc = static_cast<int>(n);

    if (tc == 1) {
        for (auto& x : data) transform(x);
        return;
    }

    std::exception_ptr firstException = nullptr;
    std::mutex exMutex;

    auto worker = [&](std::size_t begin, std::size_t end) {
        try {
            for (std::size_t i = begin; i < end; ++i) transform(data[i]);
        } catch (...) {
            std::lock_guard<std::mutex> lock(exMutex);
            if (!firstException) firstException = std::current_exception();
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(static_cast<std::size_t>(tc));

    const std::size_t base = n / static_cast<std::size_t>(tc);
    const std::size_t rem  = n % static_cast<std::size_t>(tc);

    std::size_t pos = 0;
    for (int t = 0; t < tc; ++t) {
        const std::size_t len = base + (static_cast<std::size_t>(t) < rem ? 1u : 0u);
        const std::size_t begin = pos;
        const std::size_t end   = pos + len;
        pos = end;
        threads.emplace_back(worker, begin, end);
    }

    for (auto& th : threads) th.join();

    if (firstException) std::rethrow_exception(firstException);
}