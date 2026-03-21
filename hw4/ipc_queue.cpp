#include "ipc_queue.h"

#include <cstring>

namespace hw4 {

void init_queue(SharedQueueLayout* queue) {
    queue->meta.magic = PROTOCOL_MAGIC;
    queue->meta.version = PROTOCOL_VERSION;
    queue->meta.capacity = QUEUE_CAPACITY;
    queue->meta.max_payload_size = MAX_PAYLOAD_SIZE;

    queue->write_index.store(0);
    queue->read_index.store(0);

    for (std::size_t i = 0; i < QUEUE_CAPACITY; ++i) {
        queue->slots[i].state.store(static_cast<std::uint32_t>(SlotState::EMPTY));
        queue->slots[i].header.type = static_cast<std::uint32_t>(MessageType::UNKNOWN);
        queue->slots[i].header.length = 0;
        std::memset(queue->slots[i].payload, 0, MAX_PAYLOAD_SIZE);
    }
}

bool is_queue_valid(const SharedQueueLayout* queue) {
    if (queue->meta.magic != PROTOCOL_MAGIC) {
        return false;
    }

    if (queue->meta.version != PROTOCOL_VERSION) {
        return false;
    }

    if (queue->meta.capacity != QUEUE_CAPACITY) {
        return false;
    }

    if (queue->meta.max_payload_size != MAX_PAYLOAD_SIZE) {
        return false;
    }

    return true;
}

MessageSlot* get_slot(SharedQueueLayout* queue, std::size_t index) {
    return &queue->slots[index % QUEUE_CAPACITY];
}

const MessageSlot* get_slot(const SharedQueueLayout* queue, std::size_t index) {
    return &queue->slots[index % QUEUE_CAPACITY];
}

bool try_push_message(SharedQueueLayout* queue,
                      MessageType type,
                      const void* data,
                      std::size_t length) {
    if (queue == nullptr || data == nullptr) {
        return false;
    }

    if (length > MAX_PAYLOAD_SIZE) {
        return false;
    }

    std::size_t index = queue->write_index.fetch_add(1, std::memory_order_relaxed);
    MessageSlot* slot = get_slot(queue, index);

    std::uint32_t expected = static_cast<std::uint32_t>(SlotState::EMPTY);
    if (!slot->state.compare_exchange_strong(
            expected,
            static_cast<std::uint32_t>(SlotState::WRITING),
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        return false;
    }

    slot->header.type = static_cast<std::uint32_t>(type);
    slot->header.length = static_cast<std::uint32_t>(length);

    if (length > 0) {
        std::memcpy(slot->payload, data, length);
    }

    slot->state.store(static_cast<std::uint32_t>(SlotState::READY),
                      std::memory_order_release);

    return true;
}

bool try_pop_message(SharedQueueLayout* queue,
                     MessageType expected_type,
                     char* out_buffer,
                     std::size_t buffer_size,
                     MessageHeader* out_header) {
    if (queue == nullptr || out_buffer == nullptr || out_header == nullptr) {
        return false;
    }

    std::size_t index = queue->read_index.load(std::memory_order_relaxed);
    MessageSlot* slot = get_slot(queue, index);

    std::uint32_t state = slot->state.load(std::memory_order_acquire);
    if (state != static_cast<std::uint32_t>(SlotState::READY)) {
        return false;
    }

    MessageHeader header = slot->header;

    if (header.length > MAX_PAYLOAD_SIZE) {
        return false;
    }

    if (header.length > buffer_size) {
        return false;
    }

    if (header.type == static_cast<std::uint32_t>(expected_type)) {
        if (header.length > 0) {
            std::memcpy(out_buffer, slot->payload, header.length);
        }
        *out_header = header;
    }

    slot->header.type = static_cast<std::uint32_t>(MessageType::UNKNOWN);
    slot->header.length = 0;
    std::memset(slot->payload, 0, MAX_PAYLOAD_SIZE);

    slot->state.store(static_cast<std::uint32_t>(SlotState::EMPTY),
                      std::memory_order_release);

    queue->read_index.store(index + 1, std::memory_order_relaxed);

    if (header.type != static_cast<std::uint32_t>(expected_type)) {
        return false;
    }

    return true;
}

}
