#pragma once

#include <linux/futex.h>
#include <sys/syscall.h>

#include <atomic>

static void FutexWait(void* value, int expected) {
    syscall(SYS_futex, value, FUTEX_WAIT_PRIVATE, expected, nullptr, nullptr, 0);
}

static void FutexWake(void* value, int count) {
    syscall(SYS_futex, value, FUTEX_WAKE_PRIVATE, count, nullptr, nullptr, 0);
}

class Mutex {
public:
    void Lock() {
        int expected = 0;
        while (!state_.compare_exchange_weak(expected, 1, std::memory_order_acquire)) {
            expected = 0;
            FutexWait(&state_, 1);
        }
    }

    void Unlock() {
        state_.store(0, std::memory_order_release);
        FutexWake(&state_, 1);
    }

private:
    std::atomic<int> state_{0};
};
