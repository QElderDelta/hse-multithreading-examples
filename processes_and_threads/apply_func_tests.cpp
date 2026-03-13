#include "apply_func.h"

#include <gtest/gtest.h>
#include <vector>

TEST(ApplyFunctionTest, EmptyVector) {
    std::vector<int> data;

    ApplyFunction<int>(data, [](int& x) { x += 1; }, 4);

    EXPECT_TRUE(data.empty());
}

TEST(ApplyFunctionTest, MultiplyElements) {
    std::vector<int> data = {1, 2, 3, 4};

    ApplyFunction<int>(data, [](int& x) { x *= 2; }, 2);

    EXPECT_EQ(data[0], 2);
    EXPECT_EQ(data[1], 4);
    EXPECT_EQ(data[2], 6);
    EXPECT_EQ(data[3], 8);
}

TEST(ApplyFunctionTest, SingleThread) {
    std::vector<int> data = {1, 2, 3};

    ApplyFunction<int>(data, [](int& x) { x += 5; }, 1);

    EXPECT_EQ(data[0], 6);
    EXPECT_EQ(data[1], 7);
    EXPECT_EQ(data[2], 8);
}

TEST(ApplyFunctionTest, TooManyThreads) {
    std::vector<int> data = {10, 20, 30};

    ApplyFunction<int>(data, [](int& x) { x -= 5; }, 10);

    EXPECT_EQ(data[0], 5);
    EXPECT_EQ(data[1], 15);
    EXPECT_EQ(data[2], 25);
}