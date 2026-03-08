#include <iostream>
#include <vector>
#include <thread>
#include "mutex_futex.h"

int main() {
    const int num_threads = 12;
    const int increments_per_thread = 10000;
    int shared_counter = 0;
    Mutex m;

    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < increments_per_thread; ++j) {
                    m.lock();
                    shared_counter++;
                    m.unlock();
                }
            });
        }
    }

    int expected_sum = num_threads * increments_per_thread;

    if (shared_counter == expected_sum) {
        std::cout << "SUCCESS" << std::endl;
    } else {
        std::cerr << "Data Race" << std::endl;
    }

    return 0;
}