#include <vector>
#include <thread> 
#include <functional>
#include <algorithm>

template <typename T>
void ApplyFunction(std::vector<T>& data, const std::function<void(T&)>& transform, const int threadCount = 1) {
    if (data.empty()) return;

    int actualThreads = std::min(static_cast<int>(data.size()), threadCount);
    if (actualThreads <= 1) {
        for (auto& item : data) transform(item);
        return;
    }


    std::vector<std::jthread> threads;
    size_t size = data.size();
    size_t blk_size = size / actualThreads;

    for (int i = 0; i < actualThreads; ++i) {
        size_t start = i * blk_size;
        size_t end = (i == actualThreads - 1) ? size : (i + 1) * blk_size;

        threads.emplace_back([&data, &transform, start, end]() {
            for (size_t j = start; j < end; ++j) {
                transform(data[j]);
            }
        });
    }
}