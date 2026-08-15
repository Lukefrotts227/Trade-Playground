#include "backtest/indicators.h"

namespace backtest {

std::optional<double> sma(const std::vector<Bar>& bars, size_t current_index, int period) {
    if (period <= 0 || current_index + 1 < static_cast<size_t>(period)) {
        return std::nullopt;
    }

    double sum = 0.0;
    for (size_t i = current_index + 1 - period; i <= current_index; ++i) {
        sum += bars[i].close;
    }
    return sum / period;
}

std::optional<double> ema(const std::vector<Bar>& bars, size_t current_index, int period) {
    if (period <= 0 || current_index + 1 < static_cast<size_t>(period)) {
        return std::nullopt;
    }

    size_t seed_start = current_index + 1 - period;
    double sum = 0.0;
    for (size_t i = seed_start; i < seed_start + static_cast<size_t>(period); ++i) {
        sum += bars[i].close;
    }
    double value = sum / period;

    double alpha = 2.0 / (period + 1);
    for (size_t i = seed_start + period; i <= current_index; ++i) {
        value = bars[i].close * alpha + value * (1.0 - alpha);
    }

    return value;
}

std::optional<double> rsi(const std::vector<Bar>& bars, size_t current_index, int period) {
    if (period <= 0 || current_index < static_cast<size_t>(period)) {
        return std::nullopt;
    }

    // Seed: simple average gain/loss over the first `period` price changes
    // (bars[1]-bars[0] ... bars[period]-bars[period-1]).
    double avg_gain = 0.0;
    double avg_loss = 0.0;
    for (size_t i = 0; i < static_cast<size_t>(period); ++i) {
        double change = bars[i + 1].close - bars[i].close;
        if (change > 0) {
            avg_gain += change;
        } else {
            avg_loss += -change;
        }
    }
    avg_gain /= period;
    avg_loss /= period;

    // Wilder's smoothing forward from the seed to current_index.
    for (size_t i = static_cast<size_t>(period) + 1; i <= current_index; ++i) {
        double change = bars[i].close - bars[i - 1].close;
        double gain = change > 0 ? change : 0.0;
        double loss = change < 0 ? -change : 0.0;
        avg_gain = (avg_gain * (period - 1) + gain) / period;
        avg_loss = (avg_loss * (period - 1) + loss) / period;
    }

    if (avg_loss == 0.0) {
        return 100.0;
    }

    double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

}  // namespace backtest
