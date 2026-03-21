#ifndef IPC_QUEUE_H
#define IPC_QUEUE_H

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace hw4 {

const std::uint32_t PROTOCOL_MAGIC = 0x48573431;
const std::uint32_t PROTOCOL_VERSION = 1;

const std::size_t QUEUE_CAPACITY = 16;
const std::size_t MAX_PAYLOAD_SIZE = 256;

enum class MessageType : std::uint32_t {
    UNKNOWN = 0,
    TEXT = 1,
    NUMBER = 2
};

struct MessageHeader {
    std::uint32_t type;
    std::uint32_t length;
};

struct QueueMeta {
    std::uint32_t magic;
    std::uint32_t version;
    std::uint32_t capacity;
    std::uint32_t max_payload_size;
};

enum class SlotState : std::uint32_t {
    EMPTY = 0,
    WRITING = 1,
    READY = 2
};

struct MessageSlot {
    std::atomic<std::uint32_t> state;
    MessageHeader header;
    char payload[MAX_PAYLOAD_SIZE];
};

struct SharedQueueLayout {
    QueueMeta meta;
    std::atomic<std::size_t> write_index;
    std::atomic<std::size_t> read_index;
    MessageSlot slots[QUEUE_CAPACITY];
};

void init_queue(SharedQueueLayout* queue);
bool is_queue_valid(const SharedQueueLayout* queue);

MessageSlot* get_slot(SharedQueueLayout* queue, std::size_t index);
const MessageSlot* get_slot(const SharedQueueLayout* queue, std::size_t index);

bool try_push_message(SharedQueueLayout* queue,
                      MessageType type,
                      const void* data,
                      std::size_t length);

bool try_pop_message(SharedQueueLayout* queue,
                     MessageType expected_type,
                     char* out_buffer,
                     std::size_t buffer_size,
                     MessageHeader* out_header);

}

#endif
