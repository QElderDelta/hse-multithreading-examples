#include "ipc_queue.h"
#include "shm_utils.h"

#include <cstdlib>
#include <cstring>
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
    bool was_created = false;

    if (!hw4::create_or_open_shared_memory(shm_name, queue_size, &region, &was_created)) {
        std::cerr << "Failed to create or open shared memory\n";
        return 1;
    }

    auto* queue = static_cast<hw4::SharedQueueLayout*>(region.addr);

    if (was_created) {
        hw4::init_queue(queue);
    }

    if (!hw4::is_queue_valid(queue)) {
        std::cerr << "Queue is not valid\n";
        hw4::close_shared_memory(&region);
        return 1;
    }

    int number = 12345;
    bool pushed_number = hw4::try_push_message(
        queue,
        hw4::MessageType::NUMBER,
        &number,
        sizeof(number)
    );

    if (!pushed_number) {
        std::cerr << "Failed to push NUMBER message\n";
        hw4::close_shared_memory(&region);
        return 1;
    }

    const char* text = "hello from producer";
    bool pushed_text = hw4::try_push_message(
        queue,
        hw4::MessageType::TEXT,
        text,
        std::strlen(text) + 1
    );

    if (!pushed_text) {
        std::cerr << "Failed to push TEXT message\n";
        hw4::close_shared_memory(&region);
        return 1;
    }

    if (was_created) {
        std::cout << "Shared memory created and queue initialized\n";
    } else {
        std::cout << "Shared memory opened successfully\n";
    }

    std::cout << "Pushed NUMBER message: " << number << "\n";
    std::cout << "Pushed TEXT message: " << text << "\n";
    std::cout << "Queue size: " << queue_size << " bytes\n";

    hw4::close_shared_memory(&region);
    return 0;
}
