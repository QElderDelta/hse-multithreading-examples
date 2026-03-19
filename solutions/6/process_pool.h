#pragma once

#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdexcept>
#include <type_traits>

template <typename T>
class Future {
    static_assert(std::is_trivially_copyable_v<T>,
                  "ProcessPool only supports trivially copyable result types");

public:
    Future(int pipe_read_fd, pid_t pid) : fd_(pipe_read_fd), pid_(pid) {}

    Future(const Future&) = delete;
    Future& operator=(const Future&) = delete;

    Future(Future&& other) noexcept : fd_(other.fd_), pid_(other.pid_) {
        other.fd_  = -1;
        other.pid_ = -1;
    }

    ~Future() {
        if (fd_ != -1) close(fd_);
        if (pid_ != -1) waitpid(pid_, nullptr, 0);
    }

    T Get() {
        T result;
        if (read(fd_, &result, sizeof(T)) != sizeof(T))
            throw std::runtime_error("Future::Get read failed");
        close(fd_);
        fd_ = -1;
        waitpid(pid_, nullptr, 0);
        pid_ = -1;
        return result;
    }

private:
    int   fd_;
    pid_t pid_;
};

class ProcessPool {
public:
    explicit ProcessPool(int max_workers, const char* sem_name = "/process_pool")
        : sem_name_(sem_name) {
        sem_unlink(sem_name_);
        sem_ = sem_open(sem_name_, O_CREAT | O_EXCL, 0666, max_workers);
        if (sem_ == SEM_FAILED) throw std::runtime_error("sem_open failed");
    }

    ~ProcessPool() {
        sem_close(sem_);
        sem_unlink(sem_name_);
    }

    template <typename F>
    auto Submit(F task) -> Future<std::invoke_result_t<F>> {
        using T = std::invoke_result_t<F>;

        sem_wait(sem_);

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            sem_post(sem_);
            throw std::runtime_error("pipe failed");
        }

        pid_t pid = fork();
        if (pid == -1) {
            sem_post(sem_);
            throw std::runtime_error("fork failed");
        }

        if (pid == 0) {
            close(pipefd[0]);
            T result = task();
            write(pipefd[1], &result, sizeof(T));
            close(pipefd[1]);
            sem_post(sem_);
            _exit(0);
        }

        close(pipefd[1]);
        return Future<T>(pipefd[0], pid);
    }

private:
    sem_t*      sem_;
    const char* sem_name_;
};
