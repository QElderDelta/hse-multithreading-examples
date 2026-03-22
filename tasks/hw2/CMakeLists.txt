cmake_minimum_required(VERSION 3.20)
project(apply_function LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_TESTS "Build tests" ON)
option(ENABLE_BENCHMARKS "Build benchmarks" ON)

add_library(apply_function INTERFACE)
target_include_directories(apply_function INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)

if(ENABLE_TESTS)
  enable_testing()

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_executable(apply_function_tests tests/apply_function_test.cpp)
  target_link_libraries(apply_function_tests PRIVATE apply_function GTest::gtest GTest::gtest_main)
  target_compile_features(apply_function_tests PRIVATE cxx_std_17)

  include(GoogleTest)
  gtest_discover_tests(apply_function_tests)
endif()

if(ENABLE_BENCHMARKS)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    benchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
  )
  FetchContent_MakeAvailable(benchmark)

  add_executable(apply_function_bench benchmarks/apply_function_benchmark.cpp)
  target_link_libraries(apply_function_bench PRIVATE apply_function benchmark::benchmark)
  target_compile_features(apply_function_bench PRIVATE cxx_std_17)
endif()