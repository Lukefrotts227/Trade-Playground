#include "backtest/engine.h"

#include <gtest/gtest.h>

namespace {

class RecordingStrategy : public backtest::Strategy {
public:
  backtest::Signal on_bar(const std::vector<backtest::Bar>& bars, size_t current_index) override {
    seen_timestamps.push_back(bars[current_index].timestamp);
    return backtest::Signal::Hold;
  }

  std::vector<int64_t> seen_timestamps;
};

// Buys on the very first bar, then holds for the rest of the run.
class BuyAndHoldStrategy : public backtest::Strategy {
public:
  backtest::Signal on_bar(const std::vector<backtest::Bar>&, size_t current_index) override {
    if (!bought_) {
      bought_ = true;
      return backtest::Signal::Buy;
    }
    return backtest::Signal::Hold;
  }

private:
  bool bought_ = false;
};

// Buys whenever the current close is higher than the previous bar's close.
// Exercises lookback: cannot decide anything on the very first bar.
class BuysOnUptickStrategy : public backtest::Strategy {
public:
  backtest::Signal on_bar(const std::vector<backtest::Bar>& bars, size_t current_index) override {
    if (current_index == 0) {
      return backtest::Signal::Hold;
    }
    if (bars[current_index].close > bars[current_index - 1].close) {
      return backtest::Signal::Buy;
    }
    return backtest::Signal::Hold;
  }
};

}  // namespace

TEST(Engine, CallsOnBarForEveryBarInOrder) {
  std::vector<backtest::Bar> bars = {
      {1700000000, 100.0, 105.0, 99.5, 104.0, 1000},
      {1700003600, 104.0, 106.5, 103.0, 106.0, 1500},
      {1700007200, 106.0, 107.0, 104.5, 105.0, 1200},
  };

  RecordingStrategy strategy;
  backtest::Portfolio portfolio(10000.0);
  backtest::Engine engine;
  engine.run(bars, strategy, portfolio);

  ASSERT_EQ(strategy.seen_timestamps.size(), 3);
  EXPECT_EQ(strategy.seen_timestamps[0], 1700000000);
  EXPECT_EQ(strategy.seen_timestamps[1], 1700003600);
  EXPECT_EQ(strategy.seen_timestamps[2], 1700007200);
}

TEST(Engine, HandlesEmptyBarsWithoutCallingStrategy) {
  std::vector<backtest::Bar> bars;
  RecordingStrategy strategy;
  backtest::Portfolio portfolio(10000.0);
  backtest::Engine engine;
  engine.run(bars, strategy, portfolio);

  EXPECT_EQ(strategy.seen_timestamps.size(), 0);
}

TEST(Engine, BuyAndHoldStrategyBuysOnceAndTracksEquity) {
  std::vector<backtest::Bar> bars = {
      {1700000000, 100.0, 105.0, 99.5, 100.0, 1000},
      {1700003600, 100.0, 110.0, 99.0, 110.0, 1500},
  };

  BuyAndHoldStrategy strategy;
  backtest::Portfolio portfolio(1000.0);
  backtest::Engine engine;
  engine.run(bars, strategy, portfolio);

  // Bought at close of first bar (100.0) with all $1000 -> 10 units held.
  EXPECT_DOUBLE_EQ(portfolio.cash(), 0.0);
  EXPECT_DOUBLE_EQ(portfolio.position(), 10.0);
  // Equity at final bar's close (110.0): 10 units * 110.0 = 1100.
  EXPECT_DOUBLE_EQ(portfolio.equity(bars.back()), 1100.0);
}

TEST(Engine, StrategyCanLookBackAtPriorBars) {
  std::vector<backtest::Bar> bars = {
      {1700000000, 100.0, 105.0, 99.5, 100.0, 1000},  // first bar: no prior bar, must Hold
      {1700003600, 100.0, 108.0, 99.0, 105.0, 1500},  // close > prior close -> Buy
      {1700007200, 105.0, 106.0, 95.0, 95.0, 1200},   // close < prior close -> Hold
  };

  BuysOnUptickStrategy strategy;
  backtest::Portfolio portfolio(1000.0);
  backtest::Engine engine;
  engine.run(bars, strategy, portfolio);

  // Only bought on bar index 1 (close 105.0) with all $1000 -> ~9.5238 units held.
  EXPECT_DOUBLE_EQ(portfolio.cash(), 0.0);
  EXPECT_DOUBLE_EQ(portfolio.position(), 1000.0 / 105.0);
}
