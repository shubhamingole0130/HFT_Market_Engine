#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <csignal>
#include <unordered_map>
#include <chrono>
#include <boost/asio.hpp>

// Core Logic Headers
#include "MarketStats.hpp"
#include "TradeSignal.hpp"

// Logging Headers
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// =================================================================================
// ARCHITECTURE CONFIGURATION
// Uncomment the line below to enable the Low-Latency Lock-Free Ring Buffer.
// Comment it out to revert to the standard OS-blocking Mutex Queue.
// =================================================================================
#define ENABLE_LOCK_FREE 

#ifdef ENABLE_LOCK_FREE
#include "LockFreeQueue.hpp"
#else
#include "LockQueue.hpp"
#endif

// Ensure strict memory alignment for network binary compatibility
#pragma pack(push, 1) 
struct MarketTick {
    char symbol[8];      // 8 bytes
    double price;        // 8 bytes
    uint32_t volume;     // 4 bytes
    uint64_t timestamp;  // 8 bytes
};
#pragma pack(pop)

// --- Global Resources ---
std::atomic<bool> running{ true }; // Controls graceful shutdown

#ifdef ENABLE_LOCK_FREE
// Ring Buffer: 1024 slots, Atomic indices, Non-blocking
LockFreeQueue<MarketTick, 1024> tick_queue;
#else
// Standard Queue: Unbounded, Mutex-protected, Blocking
LockQueue<MarketTick> tick_queue;
#endif


// CONSUMER THREAD: Market Data Processing & Signal Generation

void ProcessTicks() {
    // Log the active architecture mode on startup
#ifdef ENABLE_LOCK_FREE
    spdlog::info("[Worker] Mode: LOCK-FREE (Optimized for Latency)");
#else
    spdlog::info("[Worker] Mode: MUTEX-LOCK (Optimized for Safety)");
#endif

    std::unordered_map<std::string, MarketStats> market_map;

    while (running) {
        MarketTick tick;
        bool has_data = false;

        // --- Data Retrieval Strategy ---
#ifdef ENABLE_LOCK_FREE
    // Strategy A: Busy Spin (Lowest Latency)
    // Continuously polls atomic indices to avoid OS context switching overhead.
        while (!tick_queue.pop(tick)) {
            if (!running) return;
            // In a dedicated HFT core, we do not sleep here.
        }
        has_data = true;
#else
    // Strategy B: Blocking Wait (CPU Efficient)
    // Threads yield to the OS until notified by the Condition Variable.
        tick = tick_queue.pop();
        has_data = true;
#endif
       

        if (has_data) {
            std::string symbol(tick.symbol);

            // Register new symbol if unseen
            if (market_map.find(symbol) == market_map.end()) {
                market_map.emplace(symbol, MarketStats(symbol));
                spdlog::info("[System] New Asset Tracked: {}", symbol);
            }

            MarketStats& stats = market_map.at(symbol);

            // 1. Anomaly Detection (Flash Crash Logic)
            if (stats.IsAnomaly(tick.price)) {

                // 2. Latency Profiling (Wire-to-Trigger)
                auto now = std::chrono::system_clock::now();
                auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
                    now.time_since_epoch()
                ).count();
                long long latency = now_us - tick.timestamp;

                // 3. Signal Generation
                TradeSignal signal;
                signal.symbol = symbol;
                signal.type = SignalType::SELL;
                signal.price = tick.price;
                signal.z_score = stats.GetZScore(tick.price);
                signal.timestamp = tick.timestamp;

                // 4. Execution Log
                spdlog::warn(">>> EXECUTION: {} {} @ {:.2f} | Z={:.2f} | Latency: {} us",
                    signal.TypeToString(), signal.symbol, signal.price, signal.z_score, latency);
            }

            // 5. Update Rolling Statistics
            stats.AddPrice(tick.price);
        }
    }
}

// PRODUCER THREAD: UDP Network Ingestion

int main() {
    // Hook SIGINT (Ctrl+C) for safe shutdown
    std::signal(SIGINT, [](int signal) {
        running = false;
        std::cout << "\n[System] Shutdown signal received. Stopping engine...\n";
        });

    try {
        // 1. Initialize High-Performance Async Logger
        spdlog::init_thread_pool(8192, 1);
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::async_logger>(
            "logger", stdout_sink, spdlog::thread_pool(), spdlog::async_overflow_policy::block
        );
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
        spdlog::set_pattern("[%H:%M:%S.%f] [%^%l%$] %v");

        spdlog::info(">>> HFT Engine Initializing...");

        // 2. Network Setup (Boost.Asio)
        boost::asio::io_context io_context;
        boost::asio::ip::udp::socket socket(io_context,
            boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 8080));

        spdlog::info("[Network] UDP Listener active on Port 8080");

        // 3. Launch Worker Thread
        std::thread consumer_thread(ProcessTicks);
        consumer_thread.detach();

        // 4. Main Event Loop (Producer)
        char buffer[1024];
        boost::asio::ip::udp::endpoint remote_endpoint;
        boost::system::error_code error;

        while (running) {
            size_t bytes_recvd = socket.receive_from(
                boost::asio::buffer(buffer), remote_endpoint, 0, error);

            if (error && error != boost::asio::error::message_size) {
                continue; // Skip corrupted packets
            }

            if (bytes_recvd == sizeof(MarketTick)) {
                MarketTick* tick = reinterpret_cast<MarketTick*>(buffer);

#ifdef ENABLE_LOCK_FREE
                // Non-blocking push. Drop packet if queue is full (Backpressure handling)
                if (!tick_queue.push(*tick)) {
                    // Optional: Log queue overflow in debug builds
                }
#else
                // Blocking push. Waits for mutex availability.
                tick_queue.push(*tick);
#endif
            }
        }
    }
    catch (const std::exception& e) {
        spdlog::error("[Fatal] Engine Crash: {}", e.what());
        return 1;
    }

    spdlog::info("[System] Engine Stopped Gracefully.");
    return 0;
}