#include <gtest/gtest.h>
#include "applyfunction.h"
#include <string>
#include <atomic>
#include <stdexcept>


TEST(ApplyFunctionTest, BasicIncrement) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    ApplyFunction<int>(data, [](int& x) { x += 1; }, 2);
    std::vector<int> expected = {2, 3, 4, 5, 6};
    EXPECT_EQ(data, expected);
}

TEST(ApplyFunctionTest, Thread_element) {
    std::vector<int> data = {1, 2};
    ApplyFunction<int>(data, [](int& x) { x *= 2; }, 10);
    std::vector<int> expected = {2, 4};
    EXPECT_EQ(data, expected);
}

TEST(ApplyFunctionTest, EmptyVector) {
    std::vector<int> data;
    EXPECT_NO_THROW(ApplyFunction<int>(data, [](int& x) { x++; }, 4));
}

TEST(ApplyFunctionTest, DifferentTypes) {
    // double
    std::vector<double> doubleData = {1.5, 2.5, 3.5};
    ApplyFunction<double>(doubleData, [](double& x) { x *= 2.0; }, 2);
    std::vector<double> doubleExpected = {3.0, 5.0, 7.0};
    EXPECT_EQ(doubleData, doubleExpected);
    
    // string
    std::vector<std::string> stringData = {"a", "b", "c"};
    ApplyFunction<std::string>(stringData, [](std::string& x) { x += "!"; }, 2);
    std::vector<std::string> stringExpected = {"a!", "b!", "c!"};
    EXPECT_EQ(stringData, stringExpected);
}

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};

TEST(ApplyFunctionTest, CustomType) {
    std::vector<Point> data = {{1, 2}, {3, 4}, {5, 6}};
    ApplyFunction<Point>(data, [](Point& p) { p.x *= 2; p.y *= 2; }, 2);
    std::vector<Point> expected = {{2, 4}, {6, 8}, {10, 12}};
    EXPECT_EQ(data, expected);
}

TEST(ApplyFunctionTest, ThreadCNT) {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> expected = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    
    auto data1 = data;
    ApplyFunction<int>(data1, [](int& x) { x += 1; }, 1);
    EXPECT_EQ(data1, expected);
    
    auto data2 = data;
    ApplyFunction<int>(data2, [](int& x) { x += 1; }, 3);
    EXPECT_EQ(data2, expected);
    
    auto data3 = data;
    ApplyFunction<int>(data3, [](int& x) { x += 1; }, 10);
    EXPECT_EQ(data3, expected);
    
    auto data4 = data;
    ApplyFunction<int>(data4, [](int& x) { x += 1; }, 0);
    EXPECT_EQ(data4, expected);
}

TEST(ApplyFunctionTest, LargeData) {
    const int size = 100000;
    std::vector<int> data(size, 1);
    std::vector<int> expected(size, 2);
    
    ApplyFunction<int>(data, [](int& x) { x += 1; }, 4);
    EXPECT_EQ(data, expected);
}

TEST(ApplyFunctionTest, PartialModification) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<bool> modified(5, false);
    
    ApplyFunction<int>(data, [&data, &modified](int& x) {
        x += 1;
        size_t index = &x - data.data();
        modified[index] = true;
    }, 2);

    for (size_t i = 0; i < modified.size(); ++i) {
        EXPECT_TRUE(modified[i]) << "Element " << i << " was not modified";
    }
}

TEST(ApplyFunctionTest, ConstFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<int> original = data;
    
    ApplyFunction<int>(data, [](int& x) {
    }, 2);
    
    EXPECT_EQ(data, original);
}

TEST(ApplyFunctionTest, NegativeNumbers) {
    std::vector<int> data = {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5};
    ApplyFunction<int>(data, [](int& x) { x = -x; }, 3);
    
    std::vector<int> expected = {5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5};
    EXPECT_EQ(data, expected);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}