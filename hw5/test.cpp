#include <iostream>
#include <unistd.h>
#include <cassert>
#include "process_pool.h"

int main() {
    ProcessPool pool(4);
    pid_t main_pid = getpid();
    auto future_pid = pool.Submit([]() {
        return (int)getpid();
    });

    int worker_pid = future_pid.get();
    if (worker_pid != main_pid) {
        std::cout << "test1: SUCCESS PID: " << worker_pid 
                  << ", Main_PID: " << main_pid << std::endl;
    } else {
        std::cerr << "test1: FAILED" << std::endl;
        return 1;
    }
    auto future_math = pool.Submit([]() {
        return 20 + 22;
    });

    if (future_math.get() == 42) {
        std::cout << "test2: SUCCESS" << std::endl;
    } else {
        std::cerr << "test2: FAILED" << std::endl;
        return 1;
    }
    return 0;
}