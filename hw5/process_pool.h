#pragma once
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <functional>
#include "future.h"

class ProcessPool {
public:
    ProcessPool(size_t threads) {}
    Future<int> Submit(std::function<int()> func) {
        auto* state = (SharedState<int>*)mmap(NULL, sizeof(SharedState<int>), 
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        new (state) SharedState<int>();

        pid_t pid = fork();
        if (pid == 0) {
            int res = func();
            state->value = res;
            state->ready.store(true, std::memory_order_release);
            exit(0);
        } else {
            pids_.push_back(pid);
            return Future<int>(state, true);
        }
    }

    ~ProcessPool() {
        for (pid_t pid : pids_) {
            waitpid(pid, nullptr, 0);
        }
    }

private:
    std::vector<pid_t> pids_;
};