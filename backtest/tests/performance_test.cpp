#include "backtest/performance.h"

#include <gtest/gtest.h>

namespace {

backtest::Bar make_bar(int64_t timestamp, double close) {
  return backtest::Bar{timestamp, close, close, close, close, 100.0};
}

}  // namespace

TEST(Performance, ComputesTotalReturnWithNoTrades) {
  backtest::Portfolio portfolio(1000.0);
  auto bar = make_bar(1, 1000.0);

  auto report = backtest::compute_performance(portfolio, bar);

  EXPECT_DOUBLE_EQ(report.total_return_pct, 0.0);
  EXPECT_EQ(report.num_trades, 0);
  EXPECT_DOUBLE_EQ(report.win_rate_pct, 0.0);
}

TEST(Performance, ComputesReturnTradesAndWinRateForProfitableRoundTrip) {
  backtest::Portfolio portfolio(1000.0);
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(1, 100.0));
  portfolio.apply_signal(backtest::Signal::Sell, make_bar(2, 150.0));

  auto report = backtest::compute_performance(portfolio, make_bar(2, 150.0));

  EXPECT_DOUBLE_EQ(report.total_return_pct, 50.0);
  EXPECT_EQ(report.num_trades, 1);
  EXPECT_DOUBLE_EQ(report.win_rate_pct, 100.0);
}

TEST(Performance, ComputesWinRateAcrossMultipleRoundTrips) {
  backtest::Portfolio portfolio(1000.0);
  // Round trip 1: loss (100 -> 90)
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(1, 100.0));
  portfolio.apply_signal(backtest::Signal::Sell, make_bar(2, 90.0));
  // Round trip 2: win (90 -> 120)
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(3, 90.0));
  portfolio.apply_signal(backtest::Signal::Sell, make_bar(4, 120.0));

  auto report = backtest::compute_performance(portfolio, make_bar(4, 120.0));

  EXPECT_EQ(report.num_trades, 2);
  EXPECT_DOUBLE_EQ(report.win_rate_pct, 50.0);
}

TEST(Performance, TracksOpenPositionInTotalReturnButNotAsCompletedTrade) {
  backtest::Portfolio portfolio(1000.0);
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(1, 100.0));

  auto report = backtest::compute_performance(portfolio, make_bar(2, 200.0));

  EXPECT_DOUBLE_EQ(report.total_return_pct, 100.0);
  EXPECT_EQ(report.num_trades, 0);
}
