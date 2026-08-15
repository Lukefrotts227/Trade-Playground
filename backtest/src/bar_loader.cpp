#include "backtest/bar_loader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace backtest {

namespace {

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

Bar parse_bar(const std::vector<std::string>& fields, int line_number) {
    if (fields.size() != 6) {
        throw std::runtime_error(
            "load_bars: expected 6 fields on line " + std::to_string(line_number) +
            ", got " + std::to_string(fields.size()));
    }

    try {
        Bar bar;
        bar.timestamp = std::stoll(fields[0]);
        bar.open = std::stod(fields[1]);
        bar.high = std::stod(fields[2]);
        bar.low = std::stod(fields[3]);
        bar.close = std::stod(fields[4]);
        bar.volume = std::stod(fields[5]);
        return bar;
    } catch (const std::exception&) {
        throw std::runtime_error("load_bars: failed to parse line " + std::to_string(line_number));
    }
}

}  // namespace

std::vector<Bar> load_bars(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("load_bars: could not open file: " + path);
    }

    std::vector<Bar> bars;
    std::string line;
    int line_number = 0;

    // Skip header row.
    std::getline(file, line);
    line_number++;

    while (std::getline(file, line)) {
        line_number++;
        if (line.empty()) {
            continue;
        }
        bars.push_back(parse_bar(split_csv_line(line), line_number));
    }

    return bars;
}

}  // namespace backtest
