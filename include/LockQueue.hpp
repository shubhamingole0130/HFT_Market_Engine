#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdint>

// Custom packed structure to eliminate memory padding gaps across network boundaries
#pragma pack(push, 1)
struct MarketTick {
    char symbol[8];     // 8 bytes Fixed-size string array
    double price;       // 8 bytes
    uint32_t volume;    // 4 bytes
    uint64_t timestamp; // 8 bytes (Microseconds)
};
#pragma pack(pop)

template <typename T>
class LockQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    void Push(const T& item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(item);
        }
        cv_.notify_one(); // Wake up worker thread immediately
    }

    bool Pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until queue has data
        cv_.wait(lock, [this]() { 
            return !queue_.empty(); 
        });

        item = queue_.front();
        queue_.pop();
        return true;
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};