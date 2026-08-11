#pragma once
#include <atomic>
#include <vector>
#include <optional>

template <typename T, size_t Size = 1024>
class LockFreeQueue
{
private:
	std::vector<T> buffer_;

	alignas(64) std::atomic<size_t> head_{ 0 };
	alignas(64) std::atomic<size_t> tail_{ 0 };

public:
	LockFreeQueue() : buffer_(Size){}

	bool push(const T& item)
	{
		const size_t curr_head = head_.load(std::memory_order_relaxed);
		const size_t next_head = (curr_head + 1) % Size;

		if (next_head == tail_.load(std::memory_order_acquire))
		{
			return false;
		}

		buffer_[curr_head] = item;

		head_.store(next_head, std::memory_order_release);
		return true;
	}

	bool pop(T& item)
	{
		const size_t curr_tail = tail_.load(std::memory_order_relaxed);

		if (curr_tail == head_.load(std::memory_order_acquire))
		{
			return false;
		}

		item = buffer_[curr_tail];

		tail_.store((curr_tail + 1) % Size, std::memory_order_release);
		return true;
	}
};