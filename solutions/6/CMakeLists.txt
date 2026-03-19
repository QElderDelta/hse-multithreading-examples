cmake_minimum_required(VERSION 3.25)

project(process_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(process_pool main.cpp)
target_link_libraries(process_pool PRIVATE Threads::Threads)

if(UNIX AND NOT APPLE)
    target_link_libraries(process_pool PRIVATE rt)
endif()
