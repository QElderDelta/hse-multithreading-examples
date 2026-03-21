#ifndef SHM_UTILS_H
#define SHM_UTILS_H

#include <cstddef>

namespace hw4 {

struct SharedMemoryRegion {
    int fd = -1;
    void* addr = nullptr;
    std::size_t size = 0;
};

bool create_shared_memory(const char* name, std::size_t size, SharedMemoryRegion* region);
bool open_shared_memory(const char* name, std::size_t size, SharedMemoryRegion* region);
void close_shared_memory(SharedMemoryRegion* region);
bool unlink_shared_memory(const char* name);

std::size_t get_queue_memory_size();

bool create_or_open_shared_memory(const char* name,
                                  std::size_t size,
                                  SharedMemoryRegion* region,
                                  bool* was_created);
                                  
}

#endif
