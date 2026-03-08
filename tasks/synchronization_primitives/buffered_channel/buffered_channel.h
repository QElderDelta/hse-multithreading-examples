#pragma once

#include <optional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size) {
        size_ch = static_cast<size_t>(size);
        closed_ch = false;
    }

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex_ch);
        if (closed_ch) {
            throw std::runtime_error("Закрытый канал");
        }
        ch_not_full.wait(lock, [this] { 
            return queue_ch.size() < size_ch || closed_ch; 
        });
        if (closed_ch) {
            throw std::runtime_error("Закрытый канал");
        }
        queue_ch.push(value);
        ch_not_empty.notify_one(); 
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mutex_ch);
        ch_not_empty.wait(lock, [this] { 
            return !queue_ch.empty() || closed_ch; 
        });
        if (queue_ch.empty() && closed_ch) {
            return std::nullopt;
        }
        T value = std::move(queue_ch.front());
        queue_ch.pop();
        ch_not_full.notify_one();
    
        return value;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mutex_ch);
        closed_ch = true;
        ch_not_full.notify_all(); 
        ch_not_empty.notify_all();
    }

private:
    std::queue<T> queue_ch;
    size_t size_ch;
    bool closed_ch;
    std::mutex mutex_ch;
    std::condition_variable ch_not_full;
    std::condition_variable ch_not_empty;
};