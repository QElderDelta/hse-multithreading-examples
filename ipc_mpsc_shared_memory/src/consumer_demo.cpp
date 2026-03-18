#include "ipc_mpsc_queue.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: consumer_demo <shm_name> <desired_type> <messages_to_receive>\n";
        return 1;
    }

    try {
        const std::string shm_name = argv[1];
        const auto desired_type = static_cast<std::uint16_t>(std::stoul(argv[2]));
        const int target_messages = std::stoi(argv[3]);

        auto node = ipc_mpsc::ConsumerNode::open(shm_name, 300, std::chrono::milliseconds(100));

        int received = 0;
        while (received < target_messages) {
            const auto event = node.read_only(desired_type);

            switch (event.status) {
                case ipc_mpsc::ReadStatus::NoData:
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    break;

                case ipc_mpsc::ReadStatus::Dropped:
                    std::cout << "dropped message with type=" << event.dropped_type << '\n';
                    break;

                case ipc_mpsc::ReadStatus::Received: {
                    const char* text = reinterpret_cast<const char*>(event.message.payload.data());
                    std::cout << "received type=" << event.message.type << ": " << text << '\n';
                    ++received;
                    break;
                }
            }
        }

        std::cout << "consumer finished after receiving " << received << " target messages\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "consumer_demo error: " << ex.what() << '\n';
        return 1;
    }
}