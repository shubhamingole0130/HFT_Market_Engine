#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <thread>
#include <vector>
#include <atomic>

// Manually define the mode for testing (Ensure this matches main.cpp)
#define ENABLE_LOCK_FREE 

#ifdef ENABLE_LOCK_FREE
#include "../include/LockFreeQueue.hpp"
#else
#include "../include/LockQueue.hpp"
#endif

struct MarketTick {
    char symbol[8];
    double price;
    uint32_t volume;
    uint64_t timestamp;
};

TEST_CASE("Hybrid Queue Core Logic") {

#ifdef ENABLE_LOCK_FREE
    // --- LOCK-FREE MODE TESTS ---
    LockFreeQueue<MarketTick, 16> queue; // Small size for testing
    MarketTick t1 = { "AAPL", 150.0, 100, 1000 };

    SUBCASE("Push and Pop Single Item") {
        CHECK(queue.push(t1) == true);

        MarketTick t2;
        CHECK(queue.pop(t2) == true);
        CHECK(t2.price == 150.0);
    }

    SUBCASE("Queue Full Behavior (Drop Packet)") {
        // Fill the queue (Size 16)
        for (int i = 0; i < 16; i++) {
            queue.push(t1);
        }
        // 17th item should fail (return false)
        CHECK(queue.push(t1) == false);
    }

#else
    // --- MUTEX MODE TESTS ---
    LockQueue<MarketTick> queue;
    MarketTick t1 = { "AAPL", 150.0, 100, 1000 };

    SUBCASE("Push and Pop") {
        queue.push(t1);
        MarketTick t2 = queue.pop();
        CHECK(t2.price == 150.0);
    }
#endif
}