#pragma once

#include <cstddef>
#include <vector>

#include "backtest/bar.h"

namespace backtest {

enum class Signal { Hold, Buy, Sell };

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual Signal on_bar(const std::vector<Bar>& bars, size_t current_index) = 0;
};

}  // namespace backtest
