#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include <cstring>
#include "../include/LockQueue.hpp"
#include "../include/MarketStats.hpp"
#include "../include/ExecutionEngine.hpp"

// Global thread-safe queue to pass network data packets between threads
LockQueue<MarketTick> marketQueue;
std::atomic<bool> keepRunning(true);

// 1. PRODUCER: Simulates raw multi-asset binary network ingestion
void NetworkIngestionWorker() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> price_dist(150.0, 1.5); 
    std::uniform_int_distribution<> vol_dist(10, 500);

    uint64_t simulated_time = 1700000000000;

    while (keepRunning) {
        MarketTick tick;
        std::memset(tick.symbol, 0, sizeof(tick.symbol));
        std::strncpy(tick.symbol, "AAPL", 4);
        
        tick.price = price_dist(gen);
        tick.volume = vol_dist(gen);
        tick.timestamp = simulated_time++;

        marketQueue.Push(tick);
        std::this_thread::sleep_for(std::chrono::milliseconds(2)); // High-speed ingest interval
    }
}

// 2. CONSUMER: Processes data, runs statistics, flags anomalies, routes trades
void AnalyticsAndExecutionWorker() {
    MarketStats aapl_stats("AAPL", 200); 
    ExecutionEngine execution_engine;
    MarketTick incoming_tick;

    std::cout << "[Engine] Consumer worker processing thread engaged.\n";

    while (keepRunning || !marketQueue.Empty()) {
        if (marketQueue.Empty()) {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            continue;
        }

        marketQueue.Pop(incoming_tick);
        aapl_stats.AddPrice(incoming_tick.price);

        // Algorithmic Trading Rules Execution
        if (aapl_stats.GetZScore(incoming_tick.price) > 2.0) { // Entry strategy threshold
            TradeSignal signal;
            signal.symbol = "AAPL";
            signal.price = incoming_tick.price;
            signal.timestamp = incoming_tick.timestamp;
            signal.z_score = aapl_stats.GetZScore(incoming_tick.price);

            // Simple mean-reversion strategy layout
            if (incoming_tick.price < aapl_stats.GetMean()) {
                signal.type = SignalType::BUY;
            } else {
                signal.type = SignalType::SELL;
            }

            execution_engine.OnSignalReceived(signal);
        }
    }

    std::cout << "\n=== Engine Metrics Summary ===\n";
    std::cout << "Total Filled Orders: " << execution_engine.GetTotalExecutedOrders() << "\n";
    std::cout << "Net Asset Portfolio Exposure: $" << execution_engine.GetCurrentExposure() << "\n";
}

int main() {
    std::cout << "=== LAUNCHING HIGH-FREQUENCY TRADING MARKET ENGINE ENGINE ===" << std::endl;

    std::thread producer(NetworkIngestionWorker);
    std::thread consumer(AnalyticsAndExecutionWorker);

    // Let the trade execution loops process ticks for 4 seconds
    std::this_thread::sleep_for(std::chrono::seconds(4));
    
    std::cout << "\n=== INITIATING GRACEFUL ENGINE SHUTDOWN CODES ===" << std::endl;
    keepRunning = false;

    producer.join();
    consumer.join();

    std::cout << "System offline. Execution logs finalized successfully." << std::endl;
    return 0;
}