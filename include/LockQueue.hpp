#pragma once
#include <queue>              
#include <mutex>              
#include <condition_variable> 

template<typename T>
class LockQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;

public:
    // Pushes an item into the queue safely
    void push(const T& item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(item);
        }
        cond_.notify_one();
    }

    // Pops an item safely. Blocks if queue is empty.
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty(); });

        T item = queue_.front();
        queue_.pop();
        return item;
    }

    size_t size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};