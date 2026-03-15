#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <climits>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace {

int FutexWait(std::atomic<int>* value, int expected) {
    return syscall(
        SYS_futex,
        reinterpret_cast<int*>(value),
        FUTEX_WAIT_PRIVATE,
        expected,
        nullptr,
        nullptr,
        0
    );
}

int FutexWakeOne(std::atomic<int>* value) {
    return syscall(
        SYS_futex,
        reinterpret_cast<int*>(value),
        FUTEX_WAKE_PRIVATE,
        1,
        nullptr,
        nullptr,
        0
    );
}

int FutexWakeAll(std::atomic<int>* value) {
    return syscall(
        SYS_futex,
        reinterpret_cast<int*>(value),
        FUTEX_WAKE_PRIVATE,
        INT_MAX,
        nullptr,
        nullptr,
        0
    );
}

class FutexConditionVariable {
public:
    void notify_one() {
        seq_.fetch_add(1, std::memory_order_release);
        FutexWakeOne(&seq_);
    }

    void notify_all() {
        seq_.fetch_add(1, std::memory_order_release);
        FutexWakeAll(&seq_);
    }

    void wait(std::unique_lock<std::mutex>& lock) {
        const int expected = seq_.load(std::memory_order_acquire);

        lock.unlock();

        while (true) {
            const int current = seq_.load(std::memory_order_acquire);
            if (current != expected) {
                break;
            }

            const int rc = FutexWait(&seq_, expected);
            if (rc == -1 && errno == EAGAIN) {
                break;
            }
            if (rc == -1 && errno == EINTR) {
                continue;
            }
        }

        lock.lock();
    }

    template <class Predicate>
    void wait(std::unique_lock<std::mutex>& lock, Predicate pred) {
        while (!pred()) {
            wait(lock);
        }
    }

private:
    std::atomic<int> seq_{0};
};

}  // namespace

int main() {
    std::mutex mutex;
    FutexConditionVariable cv;

    bool ready = false;
    int shared_value = 0;

    std::thread worker([&]() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return ready; });
        std::cout << "Worker woke up, shared_value = " << shared_value << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    {
        std::lock_guard<std::mutex> lock(mutex);
        shared_value = 42;
        ready = true;
    }

    cv.notify_one();

    worker.join();
    return 0;
}