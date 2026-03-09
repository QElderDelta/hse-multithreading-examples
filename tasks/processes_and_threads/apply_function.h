#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <algorithm>

template <typename T>
void ApplyFunction(std::vector<T>& data, const std::function<void(T&)>& transform, const int threadCount = 1) {
    if (data.empty()) return;

    int n_threads = std::max(1, threadCount);
    if (static_cast<size_t>(n_threads) > data.size()) {
        n_threads = static_cast<int>(data.size());
    }

    std::vector<std::thread> workers;
    workers.reserve(n_threads);

    size_t base_chunk = data.size() / n_threads;
    size_t extra_elements = data.size() % n_threads;

    auto it_start = data.begin();

    for (int i = 0; i < n_threads; ++i) {
        size_t current_chunk_size = base_chunk + (i < static_cast<int>(extra_elements) ? 1 : 0);
        auto it_end = it_start + current_chunk_size;

        workers.emplace_back([it_start, it_end, &transform]() {
            for (auto it = it_start; it != it_end; ++it) {
                transform(*it);
            }
        });

        it_start = it_end;
    }

    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }
}
