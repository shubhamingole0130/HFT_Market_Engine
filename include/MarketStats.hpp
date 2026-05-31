#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <spdlog/spdlog.h>

class MarketStats {
private:
    std::string symbol_;
    std::vector<double> prices_; // Stores history
    size_t max_history_;         // How many prices to keep (e.g., 1000)

    double running_sum_ = 0.0;
    double running_sq_sum_ = 0.0;

    void PrintStats(double curr_price)
    {
        if (prices_.size() % 50 == 0) {
            // INFO level = Green/White Color
            // syntax: {} are placeholders for variables
            spdlog::info("[{}] Price: {:.2f} | Mean: {:.2f} | StdDev: {:.2f}",
                symbol_, curr_price, GetMean(), GetStdDev());
        }
    }

public:
    MarketStats(std::string symbol, size_t max_history = 1000)
        : symbol_(symbol), max_history_(max_history) {
        // Reserve memory to prevent re-allocations (Optimization)
        prices_.reserve(max_history_);
    }

    void AddPrice(double price) {
       
        // 1. If we exceed history, remove the oldest (Sliding Window)
        if (prices_.size() > max_history_) 
        {
            double oldPrice = prices_.front();
            running_sum_ -= oldPrice;
            running_sq_sum_ -= (oldPrice * oldPrice);
            prices_.erase(prices_.begin()); // Remove index 0
        }

        // 2. Add new price
        prices_.push_back(price);
        running_sum_ += price;
        running_sq_sum_ += (price * price);

        // Debug: Show we are storing it
        if (prices_.size() > 10)
        {
            PrintStats(price);
        }
    }

    double GetMean() const
    {
        if (prices_.size() < 2) return 0.0;
        return running_sum_ / prices_.size();
    }

    double GetStdDev() const
    {
        if (prices_.size() < 2) return 0.0;

        double mean = GetMean();

        double variance = (running_sq_sum_ / prices_.size()) - (mean * mean);

        if (variance < 0) variance = 0;

        return std::sqrt(variance);
    }
    
    double GetZScore(double curr_price) const
    {
        double std_dev = GetStdDev();

        if (std_dev < 0.001) return 0.0;

        return std::abs(curr_price - GetMean()) / std_dev;
    }
    
    bool IsAnomaly(double curr_price, double threshold = 3.0) const 
    {
        double z = GetZScore(curr_price);
        if (z > threshold)
        {
            return true;
        }
        return false;
    }
};