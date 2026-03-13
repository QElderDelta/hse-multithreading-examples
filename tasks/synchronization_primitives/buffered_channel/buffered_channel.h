#pragma once

#include <optional>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <stdexcept>

template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size)
        : capacity_(size) {
    }

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);

        // ждём пока появится место в буфере
        can_send_.wait(lock, [this]() {
            return closed_ || static_cast<int>(queue_.size()) < capacity_;
        });

        if (closed_) {
            throw std::runtime_error("channel is closed");
        }

        // кладём элемент в очередь
        queue_.push(value);

        // появился новый элемент → можно будить Recv
        can_recv_.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mutex_);

        // ждём пока появится элемент
        can_recv_.wait(lock, [this]() {
            return closed_ || !queue_.empty();
        });

        // если канал закрыт и очередь уже пустая
        if (queue_.empty()) {
            return std::nullopt;
        }

        // берём первый элемент
        T value = queue_.front();
        queue_.pop();

        // освободилось место → можно будить Send
        can_send_.notify_one();

        return value;
    }

    void Close() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;

        // разбудить всех ожидающих потоков
        can_send_.notify_all();
        can_recv_.notify_all();
    }

private:
    int capacity_;      // максимальный размер буфера
    bool closed_ = false;

    std::queue<T> queue_;  // FIFO буфер

    std::mutex mutex_;
    std::condition_variable can_send_;
    std::condition_variable can_recv_;
};