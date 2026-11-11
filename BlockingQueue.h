#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>

class BlockingQueue {
public:
    bool push(int value);
    std::optional<int> pop();
    void close();

private:
    std::queue<int> items_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool closed_ = false;
};