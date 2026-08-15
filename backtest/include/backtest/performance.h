#pragma once

#include "backtest/bar.h"
#include "backtest/portfolio.h"

namespace backtest {

struct PerformanceReport {
    double total_return_pct;
    int num_trades;
    double win_rate_pct;
};

PerformanceReport compute_performance(const Portfolio& portfolio, const Bar& final_bar);

}  // namespace backtest
