#pragma once

#include <functional>
#include <thread>
#include <vector>

template <typename T>
void ApplyFunction(std::vector<T>& data,
                   const std::function<void(T&)>& transform,
                   const int threadCount = 1) {
    if (data.empty()) {
        return;
    }

    int actualThreads = threadCount;
    if (actualThreads <= 0) {
        actualThreads = 1;
    }
    if (actualThreads > static_cast<int>(data.size())) {
        actualThreads = static_cast<int>(data.size());
    }

    if (actualThreads == 1) {
        for (T& element : data) {
            transform(element);
        }
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(actualThreads);

    std::size_t baseBlockSize = data.size() / actualThreads;
    std::size_t remainder = data.size() % actualThreads;
    std::size_t left = 0;

    for (int i = 0; i < actualThreads; ++i) {
        std::size_t currentBlockSize = baseBlockSize;
        if (static_cast<std::size_t>(i) < remainder) {
            ++currentBlockSize; // первые потоки берут по одному лишнему эл.
        }

        std::size_t right = left + currentBlockSize;

        threads.emplace_back([left, right, &data, &transform]() {
            for (std::size_t j = left; j < right; ++j) {
                transform(data[j]);
            }
        });

        left = right;
    }

    for (std::thread& t : threads) {
        t.join(); // ждём завершения всех потоков
    }
}