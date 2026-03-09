#include "message_queue.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <new>
#include <stdexcept>

MessageQueue::MessageQueue(bool producer, size_t buffer_size) : m_producer(producer) {
    int flag = producer ? (O_CREAT | O_RDWR) : O_RDWR;
    int shm_fd = shm_open(SHM_NAME, flag, 0666);
    if (shm_fd == -1) throw std::runtime_error("shm_open не сработал");

    if (producer) {
        m_size = sizeof(QueueData) + buffer_size;
        ftruncate(shm_fd, m_size);
    } else {
        QueueData tmp;
        read(shm_fd, &tmp, sizeof(QueueData));
        m_size = sizeof(QueueData) + tmp.max_size;
    }

    m_mmap_ptr = mmap(nullptr, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    m_data = static_cast<QueueData*>(m_mmap_ptr);
    m_buffer = static_cast<uint8_t*>(m_mmap_ptr) + sizeof(QueueData);

    if (producer) {
        new (m_data) QueueData();
        m_data->version = PROTOCOL_VERSION;
        m_data->max_size = buffer_size;
        m_data->head.store(0);
        m_data->tail.store(0);
        m_data->ready_tail.store(0);
        m_sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    } else {
        if (m_data->version != PROTOCOL_VERSION) throw std::runtime_error("Version mismatch");
        m_sem = sem_open(SEM_NAME, 0);
    }
    close(shm_fd);
}

bool MessageQueue::Send(uint32_t type, const void* data, uint32_t len) {
    size_t sum_m_len = sizeof(MessageHeader) + len;
    size_t curr_tail, next_tail;

    do {
        curr_tail = m_data->tail.load(std::memory_order_relaxed);
        size_t curr_head = m_data->head.load(std::memory_order_acquire);
        if (curr_tail + sum_m_len - curr_head > m_data->max_size) {
            return false; 
        }
        next_tail = curr_tail + sum_m_len;
    } while (!m_data->tail.compare_exchange_weak(curr_tail, next_tail, std::memory_order_relaxed));
    size_t offset = curr_tail % m_data->max_size;
    MessageHeader header{type, (uint32_t)len};
    std::memcpy(m_buffer + offset, &header, sizeof(header));
    std::memcpy(m_buffer + offset + sizeof(header), data, len);
    while (m_data->ready_tail.load(std::memory_order_acquire) != curr_tail) {}
    m_data->ready_tail.store(next_tail, std::memory_order_release);
    sem_post(m_sem);
    return true;
}

void MessageQueue::Subscribe(uint32_t type) { m_filters.insert(type); }

std::vector<uint8_t> MessageQueue::Receive() {
    while (true) {
        size_t curr_head = m_data->head.load(std::memory_order_relaxed);
        if (curr_head == m_data->ready_tail.load(std::memory_order_acquire)) {
            sem_wait(m_sem);
            continue;
        }

        size_t offset = curr_head % m_data->max_size;
        MessageHeader header;
        std::memcpy(&header, m_buffer + offset, sizeof(header));
        size_t next_head = curr_head + sizeof(MessageHeader) + header.len;

        if (m_filters.count(header.type)) {
            std::vector<uint8_t> res(header.len);
            std::memcpy(res.data(), m_buffer + offset + sizeof(MessageHeader), header.len);
            m_data->head.store(next_head, std::memory_order_relaxed);
            return res;
        }
        m_data->head.store(next_head, std::memory_order_relaxed);
    }
}

MessageQueue::~MessageQueue() {
    sem_close(m_sem);
    if (m_producer) {
        sem_unlink(SEM_NAME);
        shm_unlink(SHM_NAME);
    }
    munmap(m_mmap_ptr, m_size);
}