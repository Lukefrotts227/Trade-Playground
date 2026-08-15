#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include "backtest/bar_loader.h"
#include "backtest/engine.h"
#include "backtest/performance.h"
#include "backtest/portfolio.h"
#include "backtest/rule_strategy.h"

// Usage: backtest_cli <bars_csv_path> <starting_cash>
// Reads a JSON strategy rule from stdin, writes a PerformanceReport as JSON to stdout.
int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: backtest_cli <bars_csv_path> <starting_cash>\n";
        return 1;
    }

    std::string bars_csv_path = argv[1];
    double starting_cash = std::stod(argv[2]);

    std::stringstream rule_input;
    rule_input << std::cin.rdbuf();

    try {
        nlohmann::json rule_json = nlohmann::json::parse(rule_input.str());

        std::vector<backtest::Bar> bars = backtest::load_bars(bars_csv_path);
        if (bars.empty()) {
            std::cerr << "No bars loaded from " << bars_csv_path << "\n";
            return 1;
        }

        backtest::RuleStrategy strategy(rule_json);
        backtest::Portfolio portfolio(starting_cash);
        backtest::Engine engine;
        engine.run(bars, strategy, portfolio);

        backtest::PerformanceReport report = backtest::compute_performance(portfolio, bars.back());

        nlohmann::json output = {
            {"total_return_pct", report.total_return_pct},
            {"num_trades", report.num_trades},
            {"win_rate_pct", report.win_rate_pct},
        };

        std::cout << output.dump() << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
