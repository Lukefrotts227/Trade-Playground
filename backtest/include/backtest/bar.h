#pragma once

#include <cstdint>

namespace backtest {

struct Bar {
    int64_t timestamp;  // Unix epoch (seconds), UTC
    double open;
    double high;
    double low;
    double close;
    double volume;
};

}  // namespace backtest
