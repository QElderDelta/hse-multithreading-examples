#pragma once
#include <iostream>
#include <atomic>
#include <memory>
#include <optional>
#include <exception>
#include <thread>

template<typename T>
struct SharedState {
    T value;
    std::exception_ptr exception;
    std::atomic<bool> ready{false};
};

template<typename T>
class Future {
public:
    Future(SharedState<T>* state, bool is_shm = false) 
        : state_(state), is_shared_mem_(is_shm) {}

    T get() {
        while (!state_->ready.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        if (state_->exception) {
            std::rethrow_exception(state_->exception);
        }
        return state_->value;
    }

private:
    SharedState<T>* state_;
    bool is_shared_mem_;
};

template<typename T>
class Promise {
public:
    Promise() : state_(std::make_shared<SharedState<T>>()) {}
    Future<T> get_future() { return Future<T>(state_.get()); }

    void set_value(T val) {
        state_->value = std::move(val);
        state_->ready.store(true, std::memory_order_release);
    }

    void set_exception(std::exception_ptr e) {
        state_->exception = e;
        state_->ready.store(true, std::memory_order_release);
    }

private:
    std::shared_ptr<SharedState<T>> state_;
};