#include "shared_queue.h"

#include <chrono>
#include <cstdio>
#include <thread>

int main() {
    ProducerNode producer("/mpsc_demo");

    for (int i = 0; i < 10; ++i) {
        uint32_t type = (i % 2 == 0) ? 1 : 2;
        char buf[64];
        int len = std::snprintf(buf, sizeof(buf), "message #%d (type %u)", i, type);
        producer.Send(type, buf, len + 1);
        std::printf("[producer] sent type=%u: %s\n", type, buf);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return 0;
}
