#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include "TradeSignal.hpp"

enum class OrderStatus {
    PENDING,
    FILLED,
    REJECTED
};

struct ExecutionOrder {
    std::string order_id;
    std::string symbol;
    std::string side; // "BUY" or "SELL"
    double quantity;
    double execution_price;
    OrderStatus status;
};

class OrderRouter {
private:
    uint64_t order_sequence_ = 0;

    std::string GenerateOrderID() {
        return "ORD-" + std::to_string(++order_sequence_) + "-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count() % 100000);
    }

public:
    ExecutionOrder RouteOrder(const TradeSignal& signal, double default_qty = 100.0) {
        ExecutionOrder order;
        order.order_id = GenerateOrderID();
        order.symbol = signal.symbol;
        order.side = const_cast<TradeSignal&>(signal).TypeToString();
        order.quantity = default_qty;
        order.execution_price = signal.price; // Slippage simulation can be injected here
        
        if (signal.type == SignalType::HOLD) {
            order.status = OrderStatus::REJECTED;
        } else {
            order.status = OrderStatus::FILLED;
            std::cout << "[Order Router] >>> PLACED EXECUTION: " << order.side 
                      << " " << order.quantity << " shares of " << order.symbol 
                      << " @ $" << order.execution_price << " | ID: " << order.order_id << "\n";
        }
        
        return order;
    }
};