cmake_minimum_required(VERSION 3.10)

project(hw4_ipc_queue)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(producer_hw4
    producer_hw4.cpp
    ipc_queue.cpp
    shm_utils.cpp
)

add_executable(consumer_hw4
    consumer_hw4.cpp
    ipc_queue.cpp
    shm_utils.cpp
)
