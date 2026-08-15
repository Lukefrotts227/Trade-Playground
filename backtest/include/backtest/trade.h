#pragma once

#include <cstdint>

namespace backtest {

enum class TradeSide { Buy, Sell };

struct Trade {
    int64_t timestamp;
    TradeSide side;
    double price;
    double quantity;
};

}  // namespace backtest
