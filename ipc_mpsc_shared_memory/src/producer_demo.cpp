#include "ipc_mpsc_queue.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

namespace {

void print_usage() {
    std::cerr
        << "Usage:\n"
        << "  producer_demo --init <shm_name> <total_bytes> <producer_id> <type> <count> <delay_ms>\n"
        << "  producer_demo <shm_name> <producer_id> <type> <count> <delay_ms>\n";
}

} 

int main(int argc, char** argv) {
    try {
        bool init = false;
        int index = 1;

        if (argc > 1 && std::string(argv[1]) == "--init") {
            init = true;
            ++index;
        }

        const int expected = init ? 8 : 6;
        if (argc != expected) {
            print_usage();
            return 1;
        }

        const std::string shm_name = argv[index++];
        std::size_t total_bytes = 0;
        if (init) {
            total_bytes = static_cast<std::size_t>(std::stoull(argv[index++]));
        }

        const std::string producer_id = argv[index++];
        const auto type = static_cast<std::uint16_t>(std::stoul(argv[index++]));
        const int count = std::stoi(argv[index++]);
        const int delay_ms = std::stoi(argv[index++]);

        ipc_mpsc::ProducerNode node = init
            ? ipc_mpsc::ProducerNode::create(shm_name, total_bytes)
            : ipc_mpsc::ProducerNode::open(shm_name);

        std::cout << "producer started: id=" << producer_id << ", type=" << type << '\n';

        for (int i = 0; i < count; ++i) {
            std::string text = "producer=" + producer_id + "; seq=" + std::to_string(i) + "; type=" + std::to_string(type);

            while (!node.send(type, text.data(), static_cast<std::uint32_t>(text.size() + 1))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            std::cout << "sent: " << text << '\n';
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "producer_demo error: " << ex.what() << '\n';
        return 1;
    }
}