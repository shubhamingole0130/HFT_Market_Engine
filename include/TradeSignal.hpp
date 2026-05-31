#pragma once
#include <string>
#include <cstdint>
enum class SignalType {
    BUY,
    SELL,
    HOLD
};

struct TradeSignal {
    std::string symbol;
    SignalType type;
    double price;

    double z_score;

    uint64_t timestamp;

    std::string TypeToString() const {
        switch (type) {
        case SignalType::BUY: return "BUY";
        case SignalType::SELL: return "SELL";
        default: return "HOLD";
        }
    }
};