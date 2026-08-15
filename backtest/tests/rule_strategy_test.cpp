#include "backtest/rule_strategy.h"

#include <gtest/gtest.h>

#include "backtest/engine.h"
#include "backtest/portfolio.h"

using nlohmann::json;

namespace {

backtest::Bar make_bar(double close) {
  return backtest::Bar{1700000000, close, close, close, close, 100.0};
}

}  // namespace

TEST(RuleStrategy, BuysWhenPriceConditionIsMet) {
  json rule = {
      {"name", "buy above 100"},
      {"buy_when", json::array({{{"left", {{"type", "price"}, {"field", "close"}}},
                                  {"operator", "gt"},
                                  {"right", {{"type", "constant"}, {"value", 100.0}}}}})},
      {"sell_when", json::array()},
  };

  backtest::RuleStrategy strategy(rule);
  std::vector<backtest::Bar> bars = {make_bar(90.0), make_bar(150.0)};

  EXPECT_EQ(strategy.on_bar(bars, 0), backtest::Signal::Hold);
  EXPECT_EQ(strategy.on_bar(bars, 1), backtest::Signal::Buy);
}

TEST(RuleStrategy, HoldsWhenIndicatorHasInsufficientHistory) {
  json rule = {
      {"name", "rsi reversal"},
      {"buy_when", json::array({{{"left", {{"type", "indicator"}, {"name", "rsi"}, {"period", 14}}},
                                  {"operator", "lt"},
                                  {"right", {{"type", "constant"}, {"value", 30.0}}}}})},
      {"sell_when", json::array()},
  };

  backtest::RuleStrategy strategy(rule);
  std::vector<backtest::Bar> bars = {make_bar(100.0), make_bar(99.0)};

  // Not enough history for a 14-period RSI -> the condition can't be true -> Hold.
  EXPECT_EQ(strategy.on_bar(bars, 1), backtest::Signal::Hold);
}

TEST(RuleStrategy, EndToEndThroughEngineAndPortfolio) {
  json rule = {
      {"name", "sma crossover-ish"},
      {"buy_when", json::array({{{"left", {{"type", "price"}, {"field", "close"}}},
                                  {"operator", "gt"},
                                  {"right", {{"type", "indicator"}, {"name", "sma"}, {"period", 2}}}}})},
      {"sell_when", json::array({{{"left", {{"type", "price"}, {"field", "close"}}},
                                   {"operator", "lt"},
                                   {"right", {{"type", "indicator"}, {"name", "sma"}, {"period", 2}}}}})},
  };

  std::vector<backtest::Bar> bars = {
      make_bar(100.0),
      make_bar(100.0),
      make_bar(110.0),  // sma(2) at idx2 = avg(100,110) = 105.0, close 110 > sma -> Buy
      make_bar(90.0),   // sma(2) at idx3 = avg(110,90) = 100.0, close 90 < sma -> Sell
  };

  backtest::RuleStrategy strategy(rule);
  backtest::Portfolio portfolio(1000.0);
  backtest::Engine engine;
  engine.run(bars, strategy, portfolio);

  EXPECT_EQ(portfolio.trades().size(), 2);
  EXPECT_TRUE(portfolio.trades()[0].side == backtest::TradeSide::Buy);
  EXPECT_TRUE(portfolio.trades()[1].side == backtest::TradeSide::Sell);
}
