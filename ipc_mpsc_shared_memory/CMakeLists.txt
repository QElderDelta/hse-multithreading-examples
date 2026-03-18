cmake_minimum_required(VERSION 3.16)
project(ipc_mpsc_shared_memory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ipc_mpsc_queue INTERFACE)
target_include_directories(ipc_mpsc_queue INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(producer_demo src/producer_demo.cpp)
target_link_libraries(producer_demo PRIVATE ipc_mpsc_queue)

add_executable(consumer_demo src/consumer_demo.cpp)
target_link_libraries(consumer_demo PRIVATE ipc_mpsc_queue)

add_executable(unlink_queue src/unlink_queue.cpp)
target_link_libraries(unlink_queue PRIVATE ipc_mpsc_queue)