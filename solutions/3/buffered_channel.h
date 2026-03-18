#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>

template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size) : capacity_(size), closed_(false) {}

    void Send(const T& value) {
        std::unique_lock lock(mutex_);
        not_full_.wait(lock, [this] {
            return closed_ || static_cast<int>(queue_.size()) < capacity_;
        });
        if (closed_) {
            throw std::runtime_error("Channel is closed");
        }
        queue_.push(value);
        not_empty_.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock lock(mutex_);
        not_empty_.wait(lock, [this] { return closed_ || !queue_.empty(); });
        if (queue_.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return value;
    }

    void Close() {
        std::unique_lock lock(mutex_);
        closed_ = true;
        not_full_.notify_all();
        not_empty_.notify_all();
    }

private:
    const int capacity_;
    bool closed_;
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
};
