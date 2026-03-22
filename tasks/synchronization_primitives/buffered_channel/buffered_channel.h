#pragma once

#include <optional>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <utility>

template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size)
        : capacity_(size) {
        if (capacity_ < 0) {
            throw std::invalid_argument("BufferedChannel: negative capacity");
        }
        if (capacity_ > 0) {
            buffer_.resize(static_cast<size_t>(capacity_));
        }
    }

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (closed_) {
            throw std::runtime_error("BufferedChannel: send on closed channel");
        }

        if (capacity_ == 0) {
            cv_not_full_.wait(lock, [&] { return closed_ || !slot_.has_value(); });
            if (closed_) {
                throw std::runtime_error("BufferedChannel: send on closed channel");
            }

            slot_ = value;
            cv_not_empty_.notify_one();

            cv_not_full_.wait(lock, [&] { return !slot_.has_value(); });
            return;
        }

        cv_not_full_.wait(lock, [&] { return closed_ || count_ < capacity_; });
        if (closed_) {
            throw std::runtime_error("BufferedChannel: send on closed channel");
        }

        buffer_[static_cast<size_t>(tail_)] = value;
        tail_ = (tail_ + 1) % capacity_;
        ++count_;

        cv_not_empty_.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mutex_);

        if (capacity_ == 0) {
            cv_not_empty_.wait(lock, [&] { return closed_ || slot_.has_value(); });
            if (!slot_.has_value()) {
                return std::nullopt;
            }

            std::optional<T> res = std::move(slot_);
            slot_.reset();
            cv_not_full_.notify_one();
            return res;
        }

        cv_not_empty_.wait(lock, [&] { return closed_ || count_ > 0; });
        if (count_ == 0) {
            return std::nullopt;
        }

        std::optional<T> res = std::move(buffer_[static_cast<size_t>(head_)]);
        buffer_[static_cast<size_t>(head_)].reset();
        head_ = (head_ + 1) % capacity_;
        --count_;

        cv_not_full_.notify_one();
        return res;
    }

    void Close() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) {
            return;
        }
        closed_ = true;
        cv_not_full_.notify_all();
        cv_not_empty_.notify_all();
    }

private:
    int capacity_{0};

    std::mutex mutex_;
    std::condition_variable cv_not_full_;
    std::condition_variable cv_not_empty_;
    bool closed_{false};

    std::vector<std::optional<T>> buffer_;
    int head_{0};
    int tail_{0};
    int count_{0};

    std::optional<T> slot_;
};
