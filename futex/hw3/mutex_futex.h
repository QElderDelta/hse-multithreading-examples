#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <atomic>
#include <iostream>

void FutexWait(void* value, int expectedValue) {
    syscall(SYS_futex, value, FUTEX_WAIT_PRIVATE, expectedValue, nullptr, nullptr, 0);
}

void FutexWake(void* value, int count) {
    syscall(SYS_futex, value, FUTEX_WAKE_PRIVATE, count, nullptr, nullptr, 0);
}

class Mutex {
public:
    Mutex() : m_state(0) {}

    void lock() {
        while (m_state.exchange(1, std::memory_order_acquire) == 1) {
            FutexWait(&m_state, 1);
        }
    }

    void unlock() {
        m_state.store(0, std::memory_order_release);
        FutexWake(&m_state, 1);
    }

private:
    std::atomic<int32_t> m_state;
};