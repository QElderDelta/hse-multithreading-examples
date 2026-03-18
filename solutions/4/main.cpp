#include "mutex.h"

#include <iostream>
#include <thread>
#include <vector>

int main() {
    Mutex mutex;
    int counter = 0;
    const int kThreads = 8;
    const int kIterations = 100000;

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < kIterations; ++j) {
                mutex.Lock();
                ++counter;
                mutex.Unlock();
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Expected: " << kThreads * kIterations << "\n";
    std::cout << "Got:      " << counter << "\n";
    std::cout << (counter == kThreads * kIterations ? "OK" : "FAIL") << "\n";
    return 0;
}
