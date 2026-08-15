#pragma once

#include <vector>

#include "backtest/bar.h"
#include "backtest/portfolio.h"
#include "backtest/strategy.h"

namespace backtest {

class Engine {
public:
    void run(const std::vector<Bar>& bars, Strategy& strategy, Portfolio& portfolio);
};

}  // namespace backtest
