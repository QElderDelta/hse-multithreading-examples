#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

#include "futex_mutex.h"

TEST(FutexMutexTest, BasicLockUnlock) {
    FutexMutex m;

    m.lock();
    m.unlock();

    SUCCEED();
}

TEST(FutexMutexTest, MutualExclusion) {
    FutexMutex m;
    int counter = 0;

    const int threads = 2;
    const int iterations = 100000;

    std::vector<std::thread> workers;

    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                m.lock();
                ++counter;
                m.unlock();
            }
        });
    }

    for (auto& t : workers) {
        t.join();
    }

    ASSERT_EQ(counter, threads * iterations);
}

TEST(FutexMutexTest, HighContention) {
    FutexMutex m;
    std::atomic<int> counter{0};

    const int threads = std::thread::hardware_concurrency();
    const int iterations = 200000;

    std::vector<std::thread> workers;

    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                m.lock();
                counter++;
                m.unlock();
            }
        });
    }

    for (auto& t : workers) {
        t.join();
    }

    ASSERT_EQ(counter.load(), threads * iterations);
}

TEST(FutexMutexTest, BlockingBehavior) {
    FutexMutex m;
    std::atomic<bool> started{false};
    std::atomic<bool> finished{false};

    m.lock();

    std::thread t([&]() {
        started = true;
        m.lock();
        finished = true;
        m.unlock();
    });

    while (!started) {}

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_FALSE(finished);

    m.unlock();

    t.join();

    ASSERT_TRUE(finished);
}

TEST(FutexMutexTest, ManyWaiters) {
    FutexMutex m;
    const int threads = 16;

    std::atomic<int> passed{0};
    std::vector<std::thread> workers;

    m.lock();

    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&]() {
            m.lock();
            passed++;
            m.unlock();
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_EQ(passed.load(), 0);

    m.unlock();

    for (auto& t : workers) {
        t.join();
    }

    ASSERT_EQ(passed.load(), threads);
}