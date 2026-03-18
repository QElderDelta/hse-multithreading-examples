#include "apply_function.h"

#include <gtest/gtest.h>

#include <numeric>
#include <vector>

TEST(ApplyFunction, EmptyVector) {
    std::vector<int> data;
    ApplyFunction<int>(data, [](int& x) { x += 1; }, 4);
    EXPECT_TRUE(data.empty());
}

TEST(ApplyFunction, SingleThread) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    ApplyFunction<int>(data, [](int& x) { x *= 2; }, 1);
    EXPECT_EQ(data, (std::vector<int>{2, 4, 6, 8, 10}));
}

TEST(ApplyFunction, MultiThread) {
    std::vector<int> data(100);
    std::iota(data.begin(), data.end(), 0);
    ApplyFunction<int>(data, [](int& x) { x *= 3; }, 4);
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(data[i], i * 3);
    }
}

TEST(ApplyFunction, ThreadCountExceedsSize) {
    std::vector<int> data = {10, 20, 30};
    ApplyFunction<int>(data, [](int& x) { x += 1; }, 100);
    EXPECT_EQ(data, (std::vector<int>{11, 21, 31}));
}
