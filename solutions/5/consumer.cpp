#include "shared_queue.h"

#include <chrono>
#include <cstdio>
#include <thread>

int main() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    ConsumerNode consumer("/mpsc_demo");

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < deadline) {
        auto msg = consumer.Recv(1);
        if (msg) {
            std::printf("[consumer] received type=1: %s\n",
                        reinterpret_cast<const char*>(msg->payload));
        }
    }
    return 0;
}
