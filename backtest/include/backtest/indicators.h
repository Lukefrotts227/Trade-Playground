#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "backtest/bar.h"

namespace backtest {

// Each of these computes the indicator's value as of bars[current_index],
// using only bars at or before that index. Returns std::nullopt if there
// is not yet enough history to compute the indicator.

std::optional<double> sma(const std::vector<Bar>& bars, size_t current_index, int period);
std::optional<double> ema(const std::vector<Bar>& bars, size_t current_index, int period);
std::optional<double> rsi(const std::vector<Bar>& bars, size_t current_index, int period);

}  // namespace backtest
