#include "process_pool.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    ProcessPool pool(3);

    std::vector<Future<int>> futures;

    for (int i = 0; i < 8; ++i) {
        futures.push_back(pool.Submit([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            return i * i;
        }));
    }

    for (int i = 0; i < 8; ++i) {
        std::cout << "Result[" << i << "] = " << futures[i].Get() << "\n";
    }

    return 0;
}
