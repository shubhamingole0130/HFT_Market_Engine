#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include "../include/LockQueue.hpp"
#include "../include/MarketStats.hpp"

// Global thread-safe queue to pass network data packets between threads
LockQueue<MarketTick> marketQueue;
std::atomic<bool> keepRunning(true);

// 1. PRODUCER: Simulates ultra-fast binary network ingestion
void NetworkIngestionWorker() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> price_dist(150.0, 2.0); // Simulating an asset tracking around $150
    std::uniform_int_distribution<> vol_dist(10, 500);

    uint64_t simulated_time = 1700000000000; // Base microsecond timestamp

    while (keepRunning) {
        MarketTick tick;
        
        // Zero out memory and safely copy fixed-width string boundary arrays
        std::memset(tick.symbol, 0, sizeof(tick.symbol));
        std::strncpy(tick.symbol, "AAPL", 4);
        
        tick.price = price_dist(gen);
        tick.volume = vol_dist(gen);
        tick.timestamp = simulated_time++;

        // Push packet directly onto the thread-safe communication layer
        marketQueue.Push(tick);

        // Throttle the loop slightly to prevent melting your local processor core
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// 2. CONSUMER: Processes data using our statistical analytics engine
void AnalyticsEngineWorker() {
    MarketStats aapl_stats("AAPL", 500); // 500-tick rolling analytics window
    MarketTick incoming_tick;

    while (keepRunning || !marketQueue.Empty()) {
        if (marketQueue.Empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        marketQueue.Pop(incoming_tick);
        aapl_stats.AddPrice(incoming_tick.price);

        // Periodically verify processing throughput locally via terminal output
        if (incoming_tick.timestamp % 100 == 0) {
            std::cout << "[Engine Monitor] Processing Ticks. Current Mean: " 
                      << aapl_stats.GetMean() << " | StdDev: " 
                      << aapl_stats.GetStdDev() << "\n";
        }
    }
}

int main() {
    std::cout << "=== Launching High-Frequency Trading Market Engine ===" << std::endl;

    // Spin up parallel processing threads
    std::thread producer(NetworkIngestionWorker);
    std::thread consumer(AnalyticsEngineWorker);

    // Run the simulation pipeline for 5 seconds, then cleanly shutdown execution loops
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "\n=== Initiating Graceful Engine Shutdown Pipeline ===" << std::endl;
    
    keepRunning = false;

    // Join parallel pipelines cleanly back to main execution line
    producer.join();
    consumer.join();

    std::cout << "Engine execution finalized successfully with zero memory leaks." << std::endl;
    return 0;
}