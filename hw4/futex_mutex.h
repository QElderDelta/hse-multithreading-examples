#pragma once

#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

class FutexMutex {
public:
    FutexMutex() : state(0) {}

    void lock() {
        int expected = 0;
        if (state.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
            return;
        }

        while (true) {
            expected = 1;
            if (state.exchange(2, std::memory_order_acquire) == 0) {
                return;
            }

            futex_wait(2);
        }
    }

    void unlock() {
        if (state.fetch_sub(1, std::memory_order_release) != 1) {
            state.store(0, std::memory_order_release);
            futex_wake();
        }
    }

private:
    std::atomic<int> state;

    void futex_wait(int expected) {
        syscall(SYS_futex, reinterpret_cast<int*>(&state), FUTEX_WAIT, expected, nullptr, nullptr, 0);
    }

    void futex_wake() {
        syscall(SYS_futex, reinterpret_cast<int*>(&state), FUTEX_WAKE, 1, nullptr, nullptr, 0);
    }
};