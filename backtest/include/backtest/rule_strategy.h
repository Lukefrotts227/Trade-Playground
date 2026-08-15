#pragma once

#include <nlohmann/json.hpp>

#include "backtest/strategy.h"

namespace backtest {

// Builds a Strategy from a JSON rule matching the schema:
// {
//   "name": "...",
//   "buy_when": [ { "left": <ValueRef>, "operator": "lt"|"lte"|"gt"|"gte"|"eq", "right": <ValueRef> }, ... ],
//   "sell_when": [ ... ]
// }
// where <ValueRef> is one of:
//   { "type": "price", "field": "open"|"high"|"low"|"close"|"volume" }
//   { "type": "indicator", "name": "sma"|"ema"|"rsi", "period": <int> }
//   { "type": "constant", "value": <number> }
class RuleStrategy : public Strategy {
public:
    explicit RuleStrategy(nlohmann::json rule);

    Signal on_bar(const std::vector<Bar>& bars, size_t current_index) override;

private:
    static double resolve_value(const nlohmann::json& value_ref, const std::vector<Bar>& bars,
                                 size_t current_index, bool& ok);
    static bool evaluate_condition(const nlohmann::json& condition, const std::vector<Bar>& bars,
                                    size_t current_index);
    static bool evaluate_all(const nlohmann::json& conditions, const std::vector<Bar>& bars,
                              size_t current_index);

    nlohmann::json rule_;
};

}  // namespace backtest
