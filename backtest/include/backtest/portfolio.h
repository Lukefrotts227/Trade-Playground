#pragma once

#include <vector>

#include "backtest/bar.h"
#include "backtest/strategy.h"
#include "backtest/trade.h"

namespace backtest {

class Portfolio {
public:
    explicit Portfolio(double starting_cash);

    void apply_signal(Signal signal, const Bar& bar);

    double cash() const;
    double position() const;
    double equity(const Bar& bar) const;
    double starting_cash() const;
    const std::vector<Trade>& trades() const;

private:
    double starting_cash_;
    double cash_;
    double position_;
    std::vector<Trade> trades_;
};

}  // namespace backtest
