#include "BlockingQueue.h"



bool BlockingQueue::push(int value) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) return false;

        items_.push(value);
    }
    cv_.notify_one();

    return true;
}



std::optional<int> BlockingQueue::pop() {

    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() {return closed_ || !items_.empty();});

    if (items_.empty()) return std::nullopt;

    int ans = items_.front();
    items_.pop();

    return ans;
}



void BlockingQueue::close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (closed_) {
            return;
        }

        closed_ = true;
    }

    cv_.notify_all();
}