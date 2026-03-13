#pragma once

#include <optional>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

template <class T>
class UnbufferedChannel {
public:
    UnbufferedChannel() = default;

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_);

        // ждём пока место свободно
        can_send_.wait(lock, [this]() {
            return closed_ || !has_value_;
        });

        if (closed_) {
            throw std::runtime_error("channel is closed");
        }

        // кладём значение
        value_ = value;
        has_value_ = true;

        // сообщаем получателю
        can_recv_.notify_one();

        // ждём пока значение заберут
        can_send_.wait(lock, [this]() {
            return closed_ || !has_value_;
        });

        if (closed_ && has_value_) {
            throw std::runtime_error("channel is closed");
        }
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mutex_);

        // ждём пока появится значение
        can_recv_.wait(lock, [this]() {
            return closed_ || has_value_;
        });

        if (!has_value_) {
            return std::nullopt;
        }

        T result = value_;
        has_value_ = false;

        can_send_.notify_one();
        return result;
    }

    void Close() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;

        can_send_.notify_all();
        can_recv_.notify_all();
    }

private:
    bool closed_ = false;
    bool has_value_ = false;
    T value_;

    std::mutex mutex_;
    std::condition_variable can_send_;
    std::condition_variable can_recv_;
};