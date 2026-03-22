#pragma once

#include <condition_variable>
#include <exception>
#include <mutex>
#include <optional>
#include <queue>

template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(std::size_t size) : capacity(size) {
    }

    void Send(const T& value) {
        std::unique_lock lock(mutex);
        sendCv.wait(lock, [this]{return isClosed || queue.size() < capacity;});
        if (isClosed) {throw std::runtime_error("Channel is closed");}
        queue.push(value);
        lock.unlock();
        receiveCv.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock lock(mutex);
        receiveCv.wait(lock, [this]{return isClosed || !queue.empty();});
        if (queue.empty()) {return std::nullopt;}
        auto value = std::move(queue.front());
        queue.pop();
        lock.unlock();
        sendCv.notify_one();
        
        return value;
    }

    void Close() {
        std::lock_guard lock(mutex);
        isClosed = true;
        sendCv.notify_all();
        receiveCv.notify_all();
    }

private:
    std::size_t capacity;
    bool isClosed = false;

    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable sendCv;
    std::condition_variable receiveCv;
};