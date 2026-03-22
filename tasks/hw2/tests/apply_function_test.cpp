#include "apply_function.h"

#include <gtest/gtest.h>
#include <atomic>
#include <stdexcept>
#include <numeric>

TEST(ApplyFunction, EmptyVector_DoesNotCallTransform)
{
    std::vector<int> data;
    std::atomic<int> calls{0};

    ApplyFunction<int>(data, [&](int&) { ++calls; }, 4);

    EXPECT_TRUE(data.empty());
    EXPECT_EQ(calls.load(), 0);
}

TEST(ApplyFunction, ThreadCountGreaterThanElements_IsClamped)
{
    std::vector<int> data{1, 2, 3};
    std::atomic<int> calls{0};

    ApplyFunction<int>(data, [&](int& x) { ++calls; x += 10; }, 100);

    EXPECT_EQ(calls.load(), 3);
    EXPECT_EQ(data, (std::vector<int>{11, 12, 13}));
}

TEST(ApplyFunction, NonPositiveThreadCount_TreatedAsSingleThread)
{
    std::vector<int> data{1, 2, 3, 4};

    ApplyFunction<int>(data, [](int& x) { x *= 2; }, 0);

    EXPECT_EQ(data, (std::vector<int>{2, 4, 6, 8}));
}

TEST(ApplyFunction, MultiThreaded_CorrectnessOnLargerVector)
{
    constexpr int N = 10000;
    std::vector<int> data(N);
    std::iota(data.begin(), data.end(), 0);

    ApplyFunction<int>(data, [](int& x) { x = x * 2 + 1; }, 8);

    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(data[i], i * 2 + 1);
    }
}

TEST(ApplyFunction, PropagatesExceptions_FromTransform)
{
    std::vector<int> data{0, 1, 2, 3, 4, 5};

    auto transform = [](int& x) {
        if (x == 3) throw std::runtime_error("boom");
        x += 1;
    };

    EXPECT_THROW(ApplyFunction<int>(data, transform, 4), std::runtime_error);
}