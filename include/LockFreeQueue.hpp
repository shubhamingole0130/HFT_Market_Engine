#pragma once
#include <vector>
#include <atomic>
#include <cstddef>

template <typename T>
class LockFreeQueue {
private:
    std::vector<T> buffer_;
    size_t capacity_;
    
    // Align atomic indices to avoid cache line "false sharing" performance degradation
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};

public:
    explicit LockFreeQueue(size_t capacity = 1024)
        : capacity_(capacity + 1) { // 1 extra slot to distinguish between empty vs full states
        buffer_.resize(capacity_);
    }

    bool Push(const T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % capacity_;

        // If the queue is full, spin-wait or drop packet (Lock-free non-blocking constraint)
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false; 
        }

        buffer_[current_tail] = item;
        tail_.store(next_tail, std::memory_order_release); // Release visibility to consumer thread
        return true;
    }

    bool Pop(T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);

        // If the queue is empty, return false immediately so the consumer can busy-spin
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false; 
        }

        item = buffer_[current_head];
        head_.store((current_head + 1) % capacity_, std::memory_order_release); // Release slot back to producer
        return true;
    }

    bool Empty() {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }
};