#include "ipc_queue.h"
#include "shm_utils.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    const char* shm_name = "/hw4_queue";
    std::size_t queue_size = hw4::get_queue_memory_size();

    if (argc > 1) {
        shm_name = argv[1];
    }

    if (argc > 2) {
        unsigned long long parsed_size = std::strtoull(argv[2], nullptr, 10);
        if (parsed_size < hw4::get_queue_memory_size()) {
            std::cerr << "Queue size is too small\n";
            return 1;
        }
        queue_size = static_cast<std::size_t>(parsed_size);
    }

    hw4::SharedMemoryRegion region{};

    if (!hw4::open_shared_memory(shm_name, queue_size, &region)) {
        std::cerr << "Failed to open shared memory\n";
        return 1;
    }

    auto* queue = static_cast<hw4::SharedQueueLayout*>(region.addr);

    if (!hw4::is_queue_valid(queue)) {
        std::cerr << "Queue is not valid\n";
        hw4::close_shared_memory(&region);
        return 1;
    }

    char buffer[hw4::MAX_PAYLOAD_SIZE] = {};
    hw4::MessageHeader header{};

    int received_text_count = 0;
    int skipped_count = 0;

    while (true) {
        std::size_t before = queue->read_index.load(std::memory_order_relaxed);

        bool popped = hw4::try_pop_message(
            queue,
            hw4::MessageType::TEXT,
            buffer,
            sizeof(buffer),
            &header
        );

        std::size_t after = queue->read_index.load(std::memory_order_relaxed);

        if (popped) {
            ++received_text_count;
            std::cout << "Received TEXT message: " << buffer << "\n";
            continue;
        }

        if (after > before) {
            ++skipped_count;
            std::cout << "Skipped message with another type\n";
            continue;
        }

        break;
    }

    std::cout << "Shared memory opened successfully\n";
    std::cout << "Queue is valid\n";
    std::cout << "Received TEXT messages: " << received_text_count << "\n";
    std::cout << "Skipped messages: " << skipped_count << "\n";
    std::cout << "Queue size: " << queue_size << " bytes\n";

    hw4::close_shared_memory(&region);

    if (hw4::unlink_shared_memory(shm_name)) {
        std::cout << "Shared memory cleaned up\n";
    } else {
        std::cout << "Failed to cleanup shared memory\n";
    }

    return 0;
}
