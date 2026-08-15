#include "backtest/indicators.h"

#include <gtest/gtest.h>

namespace {

backtest::Bar make_bar(double close) {
  return backtest::Bar{1700000000, close, close, close, close, 100.0};
}

std::vector<backtest::Bar> make_bars(std::initializer_list<double> closes) {
  std::vector<backtest::Bar> bars;
  for (double close : closes) {
    bars.push_back(make_bar(close));
  }
  return bars;
}

}  // namespace

TEST(Indicators, SmaReturnsNulloptWithInsufficientHistory) {
  auto bars = make_bars({10, 20});
  EXPECT_FALSE(backtest::sma(bars, 1, 3).has_value());
}

TEST(Indicators, SmaComputesAverageOfLastPeriodCloses) {
  auto bars = make_bars({10, 20, 30, 40, 50});
  auto result = backtest::sma(bars, 4, 3);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(*result, 40.0);
}

TEST(Indicators, EmaReturnsNulloptWithInsufficientHistory) {
  auto bars = make_bars({10, 20});
  EXPECT_FALSE(backtest::ema(bars, 1, 3).has_value());
}

TEST(Indicators, EmaMatchesHandComputedSequence) {
  auto bars = make_bars({10, 20, 30, 40, 50});
  auto result = backtest::ema(bars, 4, 3);
  ASSERT_TRUE(result.has_value());
  // Seed (SMA of first 3): 20. alpha = 0.5.
  // idx3: 40*0.5 + 20*0.5 = 30. idx4: 50*0.5 + 30*0.5 = 40.
  EXPECT_DOUBLE_EQ(*result, 40.0);
}

TEST(Indicators, RsiReturnsNulloptWithInsufficientHistory) {
  auto bars = make_bars({10, 12});
  EXPECT_FALSE(backtest::rsi(bars, 1, 2).has_value());
}

TEST(Indicators, RsiAtSeedBoundary) {
  auto bars = make_bars({10, 12, 11});
  auto result = backtest::rsi(bars, 2, 2);
  ASSERT_TRUE(result.has_value());
  // diffs: +2, -1 -> avg_gain=1, avg_loss=0.5 -> rs=2 -> rsi=100-100/3
  EXPECT_NEAR(*result, 100.0 - 100.0 / 3.0, 1e-9);
}

TEST(Indicators, RsiWithWildersSmoothingBeyondSeed) {
  auto bars = make_bars({10, 12, 11, 13, 12, 14});
  auto result = backtest::rsi(bars, 5, 2);
  ASSERT_TRUE(result.has_value());
  // Hand-computed via Wilder's smoothing: avg_gain=1.375, avg_loss=0.3125 -> rs=4.4
  EXPECT_NEAR(*result, 100.0 - 100.0 / 5.4, 1e-9);
}

TEST(Indicators, RsiIsHundredWhenNoLosses) {
  auto bars = make_bars({10, 12, 14});
  auto result = backtest::rsi(bars, 2, 2);
  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(*result, 100.0);
}
