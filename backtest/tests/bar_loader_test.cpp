#include "backtest/bar_loader.h"

#include <gtest/gtest.h>

#include <stdexcept>

namespace {

constexpr const char* kSampleCsvPath = "fixtures/sample_bars.csv";

}  // namespace

TEST(LoadBars, ParsesAllRows) {
  auto bars = backtest::load_bars(kSampleCsvPath);
  ASSERT_EQ(bars.size(), 3);
}

TEST(LoadBars, ParsesFieldsCorrectly) {
  auto bars = backtest::load_bars(kSampleCsvPath);
  const auto& first = bars[0];
  EXPECT_EQ(first.timestamp, 1700000000);
  EXPECT_DOUBLE_EQ(first.open, 100.0);
  EXPECT_DOUBLE_EQ(first.high, 105.0);
  EXPECT_DOUBLE_EQ(first.low, 99.5);
  EXPECT_DOUBLE_EQ(first.close, 104.0);
  EXPECT_DOUBLE_EQ(first.volume, 1000.0);
}

TEST(LoadBars, ThrowsOnMissingFile) {
  EXPECT_THROW(backtest::load_bars("fixtures/does_not_exist.csv"), std::runtime_error);
}
