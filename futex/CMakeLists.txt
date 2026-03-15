add_executable(futex_latch latch.cpp)
target_link_libraries(futex_latch PRIVATE Threads::Threads)
target_compile_options(futex_latch PRIVATE -fsanitize=thread)
target_link_options(futex_latch PRIVATE -fsanitize=thread)

add_executable(futex_condition_variable condition_variable.cpp)
target_link_libraries(futex_condition_variable PRIVATE Threads::Threads)
