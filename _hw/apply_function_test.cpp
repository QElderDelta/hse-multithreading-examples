#include <gtest/gtest.h>
#include "apply_function.h"
#include <cmath>
#include <string>
#include <numeric>

TEST(ApplyFunction, EmptyVector) {
    std::vector<int> data;
    ApplyFunction<int>(data, [](int& x) { x *= 2; }, 4);
    EXPECT_TRUE(data.empty());
}

TEST(ApplyFunction, SingleThread) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    ApplyFunction<int>(data, [](int& x) { x *= 3; }, 1);
    EXPECT_EQ(data, (std::vector<int>{3, 6, 9, 12, 15}));
}

TEST(ApplyFunction, MultiThread) {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
    ApplyFunction<int>(data, [](int& x) { x += 10; }, 4);
    EXPECT_EQ(data, (std::vector<int>{11, 12, 13, 14, 15, 16, 17, 18}));
}

TEST(ApplyFunction, MoreThreadsThanElements) {
    std::vector<int> data = {1, 2, 3};
    ApplyFunction<int>(data, [](int& x) { x = x * x; }, 100);
    EXPECT_EQ(data, (std::vector<int>{1, 4, 9}));
}

TEST(ApplyFunction, StringTransform) {
    std::vector<std::string> data = {"hello", "world"};
    ApplyFunction<std::string>(data, [](std::string& s) { s += "!"; }, 2);
    EXPECT_EQ(data[0], "hello!");
    EXPECT_EQ(data[1], "world!");
}

TEST(ApplyFunction, LargeVectorCorrectness) {
    const int N = 100000;
    std::vector<int> data(N);
    std::iota(data.begin(), data.end(), 0);

    std::vector<int> expected(N);
    std::iota(expected.begin(), expected.end(), 0);
    for (auto& x : expected) x = x * 2 + 1;

    ApplyFunction<int>(data, [](int& x) { x = x * 2 + 1; }, 8);
    EXPECT_EQ(data, expected);
}

TEST(ApplyFunction, DoubleTransform) {
    std::vector<double> data = {1.0, 4.0, 9.0, 16.0};
    ApplyFunction<double>(data, [](double& x) { x = std::sqrt(x); }, 2);
    for (int i = 0; i < 4; ++i)
        EXPECT_DOUBLE_EQ(data[i], static_cast<double>(i + 1));
}

TEST(ApplyFunction, ZeroThreadCountDoesNothing) {
    std::vector<int> data = {1, 2, 3};
    ApplyFunction<int>(data, [](int& x) { x = 0; }, 0);
    EXPECT_EQ(data, (std::vector<int>{1, 2, 3}));
}

TEST(ApplyFunction, ConsistencyAcrossThreadCounts) {
    for (int tc = 1; tc <= 16; ++tc) {
        std::vector<int> data(1000);
        std::iota(data.begin(), data.end(), 0);
        ApplyFunction<int>(data, [](int& x) { x = x * x; }, tc);
        for (int i = 0; i < 1000; ++i)
            EXPECT_EQ(data[i], i * i) << "threadCount=" << tc << " i=" << i;
    }
}
