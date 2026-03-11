find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_executable(apply_func_tests
    apply_func_tests.cpp
)

target_link_libraries(apply_func_tests
    GTest::gtest
    GTest::gtest_main
)

add_executable(apply_func_benchmark
    apply_func_benchmark.cpp
)

target_link_libraries(apply_func_benchmark
    benchmark::benchmark
    benchmark::benchmark_main
)

enable_testing()
add_test(NAME apply_func_tests COMMAND apply_func_tests)