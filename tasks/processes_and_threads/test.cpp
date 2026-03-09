#include <gtest/gtest.h>
#include "apply_function.h"

TEST(ApplyFunctionTest, SingleThreadExecution) {
    std::vector<int> v = {1, 2, 3};
    ApplyFunction<int>(v, [](int& x) { x *= 3; }, 1);
    EXPECT_EQ(v, std::vector<int>({3, 6, 9}));
}

TEST(ApplyFunctionTest, NormalMultiThread) {
    std::vector<int> v(100, 1);
    ApplyFunction<int>(v, [](int& x) { x += 5; }, 4);
    for(int val : v) EXPECT_EQ(val, 6);
}

TEST(ApplyFunctionTest, ExcessThreads) {
    std::vector<int> v = {1, 1};
    ApplyFunction<int>(v, [](int& x) { x += 1; }, 50);
    EXPECT_EQ(v[0], 2);
    EXPECT_EQ(v[1], 2);
}

TEST(ApplyFunctionTest, EmptyInput) {
    std::vector<int> v;
    EXPECT_NO_THROW(ApplyFunction<int>(v, [](int& x) { x++; }, 4));
    EXPECT_TRUE(v.empty());
}

TEST(ApplyFunctionTest, ZeroThreadsBoundary) {
    std::vector<int> v = {10, 20};
    ApplyFunction<int>(v, [](int& x) { x /= 10; }, 0);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST(ApplyFunctionTest, NegativeThreadsBoundary) {
    std::vector<int> v = {7};
    ApplyFunction<int>(v, [](int& x) { x += 3; }, -10);
    EXPECT_EQ(v[0], 10);
}
