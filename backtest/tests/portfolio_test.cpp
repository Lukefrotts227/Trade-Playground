#include "backtest/portfolio.h"

#include <gtest/gtest.h>

namespace {

backtest::Bar make_bar(double close) {
  return backtest::Bar{1700000000, close, close, close, close, 100.0};
}

}  // namespace

TEST(Portfolio, StartsFlatWithStartingCash) {
  backtest::Portfolio portfolio(1000.0);
  EXPECT_DOUBLE_EQ(portfolio.cash(), 1000.0);
  EXPECT_DOUBLE_EQ(portfolio.position(), 0.0);
}

TEST(Portfolio, BuySpendsAllCashAtCloseIntoPosition) {
  backtest::Portfolio portfolio(1000.0);
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(100.0));

  EXPECT_DOUBLE_EQ(portfolio.cash(), 0.0);
  EXPECT_DOUBLE_EQ(portfolio.position(), 10.0);
}

TEST(Portfolio, SellLiquidatesPositionIntoCash) {
  backtest::Portfolio portfolio(1000.0);
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(100.0));
  portfolio.apply_signal(backtest::Signal::Sell, make_bar(120.0));

  EXPECT_DOUBLE_EQ(portfolio.cash(), 1200.0);
  EXPECT_DOUBLE_EQ(portfolio.position(), 0.0);
}

TEST(Portfolio, BuyWhileAlreadyHoldingIsNoOp) {
  backtest::Portfolio portfolio(1000.0);
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(100.0));
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(50.0));

  EXPECT_DOUBLE_EQ(portfolio.cash(), 0.0);
  EXPECT_DOUBLE_EQ(portfolio.position(), 10.0);
}

TEST(Portfolio, SellWhileFlatIsNoOp) {
  backtest::Portfolio portfolio(1000.0);
  portfolio.apply_signal(backtest::Signal::Sell, make_bar(100.0));

  EXPECT_DOUBLE_EQ(portfolio.cash(), 1000.0);
  EXPECT_DOUBLE_EQ(portfolio.position(), 0.0);
}

TEST(Portfolio, EquityReflectsCurrentMarkPrice) {
  backtest::Portfolio portfolio(1000.0);
  portfolio.apply_signal(backtest::Signal::Buy, make_bar(100.0));

  EXPECT_DOUBLE_EQ(portfolio.equity(make_bar(150.0)), 1500.0);
}
