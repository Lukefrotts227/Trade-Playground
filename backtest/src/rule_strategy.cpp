#include "backtest/rule_strategy.h"

#include "backtest/indicators.h"

namespace backtest {

RuleStrategy::RuleStrategy(nlohmann::json rule) : rule_(std::move(rule)) {}

double RuleStrategy::resolve_value(const nlohmann::json& value_ref, const std::vector<Bar>& bars,
                                    size_t current_index, bool& ok) {
    ok = true;
    const std::string& type = value_ref.at("type").get_ref<const std::string&>();

    if (type == "constant") {
        return value_ref.at("value").get<double>();
    }

    if (type == "price") {
        const std::string& field = value_ref.at("field").get_ref<const std::string&>();
        const Bar& bar = bars[current_index];
        if (field == "open") return bar.open;
        if (field == "high") return bar.high;
        if (field == "low") return bar.low;
        if (field == "close") return bar.close;
        if (field == "volume") return bar.volume;
        ok = false;
        return 0.0;
    }

    if (type == "indicator") {
        const std::string& name = value_ref.at("name").get_ref<const std::string&>();
        int period = value_ref.at("period").get<int>();

        std::optional<double> result;
        if (name == "sma") {
            result = sma(bars, current_index, period);
        } else if (name == "ema") {
            result = ema(bars, current_index, period);
        } else if (name == "rsi") {
            result = rsi(bars, current_index, period);
        }

        if (!result.has_value()) {
            ok = false;
            return 0.0;
        }
        return *result;
    }

    ok = false;
    return 0.0;
}

bool RuleStrategy::evaluate_condition(const nlohmann::json& condition, const std::vector<Bar>& bars,
                                       size_t current_index) {
    bool left_ok = false;
    bool right_ok = false;
    double left = resolve_value(condition.at("left"), bars, current_index, left_ok);
    double right = resolve_value(condition.at("right"), bars, current_index, right_ok);

    if (!left_ok || !right_ok) {
        return false;
    }

    const std::string& op = condition.at("operator").get_ref<const std::string&>();
    if (op == "lt") return left < right;
    if (op == "lte") return left <= right;
    if (op == "gt") return left > right;
    if (op == "gte") return left >= right;
    if (op == "eq") return left == right;

    return false;
}

bool RuleStrategy::evaluate_all(const nlohmann::json& conditions, const std::vector<Bar>& bars,
                                 size_t current_index) {
    if (conditions.empty()) {
        return false;
    }
    for (const auto& condition : conditions) {
        if (!evaluate_condition(condition, bars, current_index)) {
            return false;
        }
    }
    return true;
}

Signal RuleStrategy::on_bar(const std::vector<Bar>& bars, size_t current_index) {
    if (evaluate_all(rule_.at("buy_when"), bars, current_index)) {
        return Signal::Buy;
    }
    if (evaluate_all(rule_.at("sell_when"), bars, current_index)) {
        return Signal::Sell;
    }
    return Signal::Hold;
}

}  // namespace backtest
