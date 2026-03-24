set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(tests tests.cpp)

target_link_libraries(tests GTest::GTest GTest::Main Threads::Threads)