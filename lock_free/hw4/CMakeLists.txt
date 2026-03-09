cmake_minimum_required(VERSION 3.20)
project(shm_queue)

set(CMAKE_CXX_STANDARD 20)

add_library(queue_lib message_qeueu.cpp)
target_link_libraries(queue_lib rt pthread)

add_executable(producer producer.cpp)
target_link_libraries(producer queue_lib)

add_executable(consumer consumer.cpp)
target_link_libraries(consumer queue_lib)