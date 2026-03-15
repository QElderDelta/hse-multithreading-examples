add_executable(false_sharing false_sharing.cpp)
target_link_libraries(false_sharing PRIVATE Threads::Threads)

#add_executable(store_buffer store_buffer.cpp)
#target_link_libraries(store_buffer PRIVATE Threads::Threads)