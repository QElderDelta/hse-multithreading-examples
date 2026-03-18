#pragma once

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

namespace ipc_mpsc {

constexpr std::uint64_t kMagic = 0x4950434d50534331ULL; 
constexpr std::uint32_t kProtocolVersion = 1;
constexpr std::size_t kAlignment = 8;
constexpr std::uint16_t kFlagWrap = 0x1;

inline std::size_t align_up(std::size_t value, std::size_t alignment = kAlignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

inline std::runtime_error make_error(const std::string& where) {
    return std::runtime_error(where + ": " + std::strerror(errno));
}

inline void validate_shm_name(const std::string& name) {
    if (name.empty() || name.front() != '/') {
        throw std::invalid_argument("shared memory name must start with '/'");
    }
}

struct QueueMeta {
    std::uint64_t magic;
    std::uint32_t protocol_version;
    std::uint32_t data_offset;
    std::uint64_t total_bytes;
    std::uint64_t capacity_bytes;
    alignas(64) std::atomic<std::uint64_t> head;
    alignas(64) std::atomic<std::uint64_t> tail;
};

struct RecordHeader {
    std::atomic<std::uint32_t> ready;
    std::uint16_t type;
    std::uint16_t flags;
    std::uint32_t payload_size;
    std::uint32_t span;
};

static_assert(sizeof(RecordHeader) == 16, "RecordHeader must stay compact");

struct Message {
    std::uint16_t type{};
    std::vector<std::byte> payload;
};

enum class ReadStatus {
    NoData,
    Received,
    Dropped,
};

struct ReadEvent {
    ReadStatus status{ReadStatus::NoData};
    Message message{};
    std::uint16_t dropped_type{};
};

class SharedMemoryRegion {
public:
    SharedMemoryRegion() = default;

    SharedMemoryRegion(int fd, void* base, std::size_t size)
        : fd_(fd), base_(base), size_(size) {
    }

    SharedMemoryRegion(const SharedMemoryRegion&) = delete;
    SharedMemoryRegion& operator=(const SharedMemoryRegion&) = delete;

    SharedMemoryRegion(SharedMemoryRegion&& other) noexcept {
        *this = std::move(other);
    }

    SharedMemoryRegion& operator=(SharedMemoryRegion&& other) noexcept {
        if (this != &other) {
            close_region();
            fd_ = other.fd_;
            base_ = other.base_;
            size_ = other.size_;
            other.fd_ = -1;
            other.base_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~SharedMemoryRegion() {
        close_region();
    }

    static SharedMemoryRegion create_or_recreate(const std::string& name, std::size_t total_bytes) {
        validate_shm_name(name);
        if (total_bytes < align_up(sizeof(QueueMeta)) + 64) {
            throw std::invalid_argument("shared memory region is too small");
        }

        ::shm_unlink(name.c_str());

        int fd = ::shm_open(name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600);
        if (fd == -1) {
            throw make_error("shm_open(create)");
        }

        if (::ftruncate(fd, static_cast<off_t>(total_bytes)) == -1) {
            int saved_errno = errno;
            ::close(fd);
            ::shm_unlink(name.c_str());
            errno = saved_errno;
            throw make_error("ftruncate");
        }

        void* base = ::mmap(nullptr, total_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (base == MAP_FAILED) {
            int saved_errno = errno;
            ::close(fd);
            ::shm_unlink(name.c_str());
            errno = saved_errno;
            throw make_error("mmap(create)");
        }

        return SharedMemoryRegion(fd, base, total_bytes);
    }

    static SharedMemoryRegion open_existing(
        const std::string& name,
        int retries = 0,
        std::chrono::milliseconds retry_delay = std::chrono::milliseconds(100)) {
        validate_shm_name(name);

        int fd = -1;
        for (int attempt = 0;; ++attempt) {
            fd = ::shm_open(name.c_str(), O_RDWR, 0600);
            if (fd != -1) {
                break;
            }
            if (errno != ENOENT || attempt >= retries) {
                throw make_error("shm_open(open)");
            }
            std::this_thread::sleep_for(retry_delay);
        }

        struct stat st {};
        if (::fstat(fd, &st) == -1) {
            int saved_errno = errno;
            ::close(fd);
            errno = saved_errno;
            throw make_error("fstat");
        }
        if (st.st_size <= 0) {
            ::close(fd);
            throw std::runtime_error("shared memory object has invalid size");
        }

        std::size_t total_bytes = static_cast<std::size_t>(st.st_size);
        void* base = ::mmap(nullptr, total_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (base == MAP_FAILED) {
            int saved_errno = errno;
            ::close(fd);
            errno = saved_errno;
            throw make_error("mmap(open)");
        }

        return SharedMemoryRegion(fd, base, total_bytes);
    }

    void* base() const {
        return base_;
    }

    std::size_t size() const {
        return size_;
    }

private:
    void close_region() {
        if (base_ != nullptr && base_ != MAP_FAILED) {
            ::munmap(base_, size_);
        }
        if (fd_ != -1) {
            ::close(fd_);
        }
        fd_ = -1;
        base_ = nullptr;
        size_ = 0;
    }

    int fd_{-1};
    void* base_{nullptr};
    std::size_t size_{0};
};

class QueueBase {
protected:
    explicit QueueBase(SharedMemoryRegion region)
        : region_(std::move(region)) {
        meta_ = reinterpret_cast<QueueMeta*>(region_.base());
        data_offset_ = align_up(sizeof(QueueMeta));
        if (region_.size() < data_offset_) {
            throw std::runtime_error("shared memory region is too small for metadata");
        }
        data_ = static_cast<std::byte*>(region_.base()) + data_offset_;
    }

    void verify_initialized() {
        if (meta_->magic != kMagic) {
            throw std::runtime_error("queue magic mismatch or queue is not initialized");
        }
        if (meta_->protocol_version != kProtocolVersion) {
            throw std::runtime_error("protocol version mismatch");
        }
        if (meta_->data_offset != data_offset_) {
            throw std::runtime_error("metadata layout mismatch");
        }
        if (meta_->total_bytes != region_.size()) {
            throw std::runtime_error("shared memory size mismatch");
        }
        capacity_ = meta_->capacity_bytes;
        if (capacity_ == 0) {
            throw std::runtime_error("queue capacity is zero");
        }
    }

    SharedMemoryRegion region_;
    QueueMeta* meta_{nullptr};
    std::byte* data_{nullptr};
    std::size_t data_offset_{0};
    std::uint64_t capacity_{0};
};

class ProducerNode : public QueueBase {
public:
    explicit ProducerNode(SharedMemoryRegion region)
        : QueueBase(std::move(region)) {
        verify_initialized();
    }

    static ProducerNode create(const std::string& name, std::size_t total_bytes) {
        auto region = SharedMemoryRegion::create_or_recreate(name, total_bytes);
        std::memset(region.base(), 0, region.size());

        auto* meta = reinterpret_cast<QueueMeta*>(region.base());
        const auto data_offset = static_cast<std::uint32_t>(align_up(sizeof(QueueMeta)));
        const auto capacity = static_cast<std::uint64_t>(region.size() - data_offset);

        meta->magic = kMagic;
        meta->protocol_version = kProtocolVersion;
        meta->data_offset = data_offset;
        meta->total_bytes = region.size();
        meta->capacity_bytes = capacity;
        meta->head.store(0, std::memory_order_relaxed);
        meta->tail.store(0, std::memory_order_relaxed);

        return ProducerNode(std::move(region));
    }

    static ProducerNode open(const std::string& name, int retries = 100) {
        return ProducerNode(SharedMemoryRegion::open_existing(name, retries));
    }

    bool send(std::uint16_t type, const void* payload, std::uint32_t payload_size) {
        const std::uint32_t message_span =
            static_cast<std::uint32_t>(align_up(sizeof(RecordHeader) + payload_size));

        if (message_span > capacity_) {
            return false;
        }

        for (;;) {
            std::uint64_t head = meta_->head.load(std::memory_order_relaxed);
            const std::uint64_t tail = meta_->tail.load(std::memory_order_acquire);
            const std::uint64_t used = head - tail;

            if (used > capacity_) {
                throw std::runtime_error("queue is corrupted: used bytes exceed capacity");
            }

            const std::uint64_t offset = head % capacity_;
            const std::uint32_t remaining = static_cast<std::uint32_t>(capacity_ - offset);
            const bool need_wrap = remaining < message_span;
            const std::uint64_t reserve = need_wrap ? (static_cast<std::uint64_t>(remaining) + message_span)
                                                    : static_cast<std::uint64_t>(message_span);

            if (reserve > capacity_ || used + reserve > capacity_) {
                return false;
            }

            if (!meta_->head.compare_exchange_weak(
                    head,
                    head + reserve,
                    std::memory_order_acq_rel,
                    std::memory_order_relaxed)) {
                continue;
            }

            if (need_wrap) {
                auto* wrap_header = reinterpret_cast<RecordHeader*>(data_ + offset);
                wrap_header->ready.store(0, std::memory_order_relaxed);
                wrap_header->type = 0;
                wrap_header->flags = kFlagWrap;
                wrap_header->payload_size = 0;
                wrap_header->span = remaining;

                write_message_at_offset(0, type, payload, payload_size, message_span);
                std::atomic_thread_fence(std::memory_order_release);
                wrap_header->ready.store(1, std::memory_order_release);
            } else {
                write_message_at_offset(offset, type, payload, payload_size, message_span);
            }

            return true;
        }
    }

private:
    void write_message_at_offset(
        std::uint64_t offset,
        std::uint16_t type,
        const void* payload,
        std::uint32_t payload_size,
        std::uint32_t span) {
        auto* header = reinterpret_cast<RecordHeader*>(data_ + offset);
        header->ready.store(0, std::memory_order_relaxed);
        header->type = type;
        header->flags = 0;
        header->payload_size = payload_size;
        header->span = span;

        if (payload_size > 0) {
            std::memcpy(data_ + offset + sizeof(RecordHeader), payload, payload_size);
        }

        std::atomic_thread_fence(std::memory_order_release);
        header->ready.store(1, std::memory_order_release);
    }
};

class ConsumerNode : public QueueBase {
public:
    explicit ConsumerNode(SharedMemoryRegion region)
        : QueueBase(std::move(region)) {
        verify_initialized();
    }

    static ConsumerNode open(
        const std::string& name,
        int retries = 300,
        std::chrono::milliseconds retry_delay = std::chrono::milliseconds(100)) {
        return ConsumerNode(SharedMemoryRegion::open_existing(name, retries, retry_delay));
    }

    ReadEvent read_only(std::uint16_t desired_type) {
        for (;;) {
            const std::uint64_t tail = meta_->tail.load(std::memory_order_relaxed);
            const std::uint64_t head = meta_->head.load(std::memory_order_acquire);

            if (tail == head) {
                return {};
            }

            const std::uint64_t offset = tail % capacity_;
            auto* header = reinterpret_cast<RecordHeader*>(data_ + offset);

            if (header->ready.load(std::memory_order_acquire) == 0) {
                return {};
            }

            const std::uint32_t span = header->span;
            if (span == 0 || span > capacity_) {
                throw std::runtime_error("queue is corrupted: invalid record span");
            }

            if ((header->flags & kFlagWrap) != 0) {
                header->ready.store(0, std::memory_order_relaxed);
                meta_->tail.store(tail + span, std::memory_order_release);
                continue;
            }

            const auto message_type = header->type;
            const auto payload_size = header->payload_size;

            if (payload_size > span - sizeof(RecordHeader)) {
                throw std::runtime_error("queue is corrupted: invalid payload size");
            }

            ReadEvent event;
            if (message_type == desired_type) {
                event.status = ReadStatus::Received;
                event.message.type = message_type;
                event.message.payload.resize(payload_size);
                if (payload_size > 0) {
                    std::memcpy(event.message.payload.data(), data_ + offset + sizeof(RecordHeader), payload_size);
                }
            } else {
                event.status = ReadStatus::Dropped;
                event.dropped_type = message_type;
            }

            header->ready.store(0, std::memory_order_relaxed);
            meta_->tail.store(tail + span, std::memory_order_release);
            return event;
        }
    }
};

inline void unlink_queue(const std::string& name) {
    validate_shm_name(name);
    if (::shm_unlink(name.c_str()) == -1 && errno != ENOENT) {
        throw make_error("shm_unlink");
    }
}

}