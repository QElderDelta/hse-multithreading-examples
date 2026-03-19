#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static constexpr uint32_t PROTOCOL_VERSION = 1;
static constexpr uint32_t MAX_PAYLOAD      = 256;
static constexpr uint32_t SLOT_COUNT       = 1024;

static_assert(std::atomic<uint32_t>::is_always_lock_free);
static_assert(std::atomic<uint8_t>::is_always_lock_free);

struct MessageHeader {
    uint32_t type;
    uint32_t length;
};

struct Message {
    MessageHeader header;
    uint8_t payload[MAX_PAYLOAD];
};

struct Slot {
    std::atomic<uint8_t> ready{0};
    Message message;
};

struct SharedMemory {
    uint32_t version;
    std::atomic<uint32_t> tail{0};
    std::atomic<uint32_t> head{0};
    Slot slots[SLOT_COUNT];
};

class ProducerNode {
public:
    ProducerNode(const char* name) : name_(name) {
        fd_ = shm_open(name, O_CREAT | O_RDWR, 0666);
        if (fd_ == -1) throw std::runtime_error("shm_open failed");
        if (ftruncate(fd_, sizeof(SharedMemory)) == -1) throw std::runtime_error("ftruncate failed");
        shm_ = static_cast<SharedMemory*>(
            mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
        if (shm_ == MAP_FAILED) throw std::runtime_error("mmap failed");

        shm_->version = PROTOCOL_VERSION;
        shm_->tail.store(0);
        shm_->head.store(0);
        for (uint32_t i = 0; i < SLOT_COUNT; ++i)
            shm_->slots[i].ready.store(0);
    }

    ~ProducerNode() {
        if (shm_ != MAP_FAILED) munmap(shm_, sizeof(SharedMemory));
        if (fd_ != -1) close(fd_);
        shm_unlink(name_);
    }

    void Send(uint32_t type, const void* data, uint32_t length) {
        if (length > MAX_PAYLOAD) throw std::runtime_error("Message too large");

        uint32_t idx = shm_->tail.fetch_add(1, std::memory_order_relaxed) % SLOT_COUNT;
        Slot& slot = shm_->slots[idx];

        while (slot.ready.load(std::memory_order_acquire) != 0) {}

        slot.message.header.type   = type;
        slot.message.header.length = length;
        std::memcpy(slot.message.payload, data, length);
        slot.ready.store(1, std::memory_order_release);
    }

private:
    int fd_ = -1;
    SharedMemory* shm_ = static_cast<SharedMemory*>(MAP_FAILED);
    const char* name_;
};

class ConsumerNode {
public:
    ConsumerNode(const char* name) {
        fd_ = shm_open(name, O_RDWR, 0666);
        if (fd_ == -1) throw std::runtime_error("shm_open failed");
        shm_ = static_cast<SharedMemory*>(
            mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
        if (shm_ == MAP_FAILED) throw std::runtime_error("mmap failed");
        if (shm_->version != PROTOCOL_VERSION)
            throw std::runtime_error("Protocol version mismatch");
    }

    ~ConsumerNode() {
        if (shm_ != MAP_FAILED) munmap(shm_, sizeof(SharedMemory));
        if (fd_ != -1) close(fd_);
    }

    std::optional<Message> Recv(int filter_type = -1) {
        while (true) {
            uint32_t idx = shm_->head.load(std::memory_order_relaxed) % SLOT_COUNT;
            Slot& slot = shm_->slots[idx];

            if (slot.ready.load(std::memory_order_acquire) == 0)
                return std::nullopt;

            Message msg = slot.message;
            slot.ready.store(0, std::memory_order_release);
            shm_->head.fetch_add(1, std::memory_order_relaxed);

            if (filter_type == -1 || static_cast<int>(msg.header.type) == filter_type)
                return msg;
        }
    }

private:
    int fd_ = -1;
    SharedMemory* shm_ = static_cast<SharedMemory*>(MAP_FAILED);
};
