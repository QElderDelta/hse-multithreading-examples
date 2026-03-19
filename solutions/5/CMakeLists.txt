cmake_minimum_required(VERSION 3.25)

project(mpsc_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(producer producer.cpp)
add_executable(consumer consumer.cpp)

if(UNIX AND NOT APPLE)
    target_link_libraries(producer PRIVATE rt)
    target_link_libraries(consumer PRIVATE rt)
endif()
