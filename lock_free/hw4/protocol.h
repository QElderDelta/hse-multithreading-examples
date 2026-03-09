#pragma once
#include <atomic>
#include <cstdint>
#include <cstddef>

constexpr uint32_t PROTOCOL_VERSION = 1;
constexpr const char* SHM_NAME = "/mpsc_queue_shm";
constexpr const char* SEM_NAME = "/mpsc_queue_sem";

struct MessageHeader {
    uint32_t type;
    uint32_t len;
};

struct QueueData {
    uint32_t version;
    uint32_t max_size;
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
    std::atomic<size_t> ready_tail;
};