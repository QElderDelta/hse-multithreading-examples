#include "ipc_mpsc_queue.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: unlink_queue <shm_name>\n";
        return 1;
    }

    try {
        ipc_mpsc::unlink_queue(argv[1]);
        std::cout << "queue removed: " << argv[1] << '\n';
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "unlink_queue error: " << ex.what() << '\n';
        return 1;
    }
}