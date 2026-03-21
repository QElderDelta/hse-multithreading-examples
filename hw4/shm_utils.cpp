#include "shm_utils.h"
#include "ipc_queue.h"

#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace hw4 {

bool create_shared_memory(const char* name, std::size_t size, SharedMemoryRegion* region) {
    if (name == nullptr || region == nullptr || size == 0) {
        return false;
    }

    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        return false;
    }

    if (ftruncate(fd, static_cast<off_t>(size)) == -1) {
        close(fd);
        return false;
    }

    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);
        return false;
    }

    region->fd = fd;
    region->addr = addr;
    region->size = size;

    return true;
}

bool open_shared_memory(const char* name, std::size_t size, SharedMemoryRegion* region) {
    if (name == nullptr || region == nullptr || size == 0) {
        return false;
    }

    int fd = shm_open(name, O_RDWR, 0666);
    if (fd == -1) {
        return false;
    }

    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);
        return false;
    }

    region->fd = fd;
    region->addr = addr;
    region->size = size;

    return true;
}

bool create_or_open_shared_memory(const char* name,
                                  std::size_t size,
                                  SharedMemoryRegion* region,
                                  bool* was_created) {
    if (name == nullptr || region == nullptr || was_created == nullptr || size == 0) {
        return false;
    }

    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0666);
    bool created_now = false;

    if (fd != -1) {
        created_now = true;

        if (ftruncate(fd, static_cast<off_t>(size)) == -1) {
            close(fd);
            shm_unlink(name);
            return false;
        }
    } else {
        if (errno != EEXIST) {
            return false;
        }

        fd = shm_open(name, O_RDWR, 0666);
        if (fd == -1) {
            return false;
        }
    }

    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);

        if (created_now) {
            shm_unlink(name);
        }

        return false;
    }

    region->fd = fd;
    region->addr = addr;
    region->size = size;
    *was_created = created_now;

    return true;
}

void close_shared_memory(SharedMemoryRegion* region) {
    if (region == nullptr) {
        return;
    }

    if (region->addr != nullptr && region->size > 0) {
        munmap(region->addr, region->size);
    }

    if (region->fd >= 0) {
        close(region->fd);
    }

    region->fd = -1;
    region->addr = nullptr;
    region->size = 0;
}

bool unlink_shared_memory(const char* name) {
    if (name == nullptr) {
        return false;
    }

    return shm_unlink(name) == 0;
}

std::size_t get_queue_memory_size() {
    return sizeof(SharedQueueLayout);
}

}
