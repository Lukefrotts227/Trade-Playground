#include "backtest/engine.h"

namespace backtest {

void Engine::run(const std::vector<Bar>& bars, Strategy& strategy, Portfolio& portfolio) {
    for (size_t i = 0; i < bars.size(); ++i) {
        Signal signal = strategy.on_bar(bars, i);
        portfolio.apply_signal(signal, bars[i]);
    }
}

}  // namespace backtest
