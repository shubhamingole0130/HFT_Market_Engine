#pragma once
#include "TradeSignal.hpp"
#include "OrderRouter.hpp"
#include <vector>

class ExecutionEngine {
private:
    OrderRouter router_;
    std::vector<ExecutionOrder> order_history_;
    double max_risk_exposure_ = 500000.0; // Max absolute dollar amount allowed active
    double current_exposure_ = 0.0;

public:
    bool PassRiskCheck(const TradeSignal& signal, double quantity) {
        double prospective_cost = signal.price * quantity;
        if (current_exposure_ + prospective_cost > max_risk_exposure_) {
            std::cout << "[Risk Engine] ⚠️ ORDER REJECTED: Risk exposure limit exceeded!\n";
            return false;
        }
        return true;
    }

    void OnSignalReceived(const TradeSignal& signal) {
        if (signal.type == SignalType::HOLD) return;

        double allocation_qty = 100.0; // Static sizing rule

        if (PassRiskCheck(signal, allocation_qty)) {
            ExecutionOrder order = router_.RouteOrder(signal, allocation_qty);
            if (order.status == OrderStatus::FILLED) {
                order_history_.push_back(order);
                if (signal.type == SignalType::BUY) {
                    current_exposure_ += (order.quantity * order.execution_price);
                } else if (signal.type == SignalType::SELL) {
                    current_exposure_ -= (order.quantity * order.execution_price);
                }
            }
        }
    }

    size_t GetTotalExecutedOrders() const { return order_history_.size(); }
    double GetCurrentExposure() const { return current_exposure_; }
};