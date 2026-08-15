#include "backtest/performance.h"

namespace backtest {

PerformanceReport compute_performance(const Portfolio& portfolio, const Bar& final_bar) {
    PerformanceReport report{};

    double final_equity = portfolio.equity(final_bar);
    report.total_return_pct = (final_equity / portfolio.starting_cash() - 1.0) * 100.0;

    const auto& trades = portfolio.trades();
    int round_trips = 0;
    int wins = 0;

    for (size_t i = 0; i + 1 < trades.size(); i += 2) {
        const Trade& entry = trades[i];
        const Trade& exit = trades[i + 1];
        if (entry.side == TradeSide::Buy && exit.side == TradeSide::Sell) {
            round_trips++;
            if (exit.price > entry.price) {
                wins++;
            }
        }
    }

    report.num_trades = round_trips;
    report.win_rate_pct = round_trips == 0 ? 0.0 : (static_cast<double>(wins) / round_trips) * 100.0;

    return report;
}

}  // namespace backtest
