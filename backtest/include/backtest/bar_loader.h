#pragma once

#include <string>
#include <vector>

#include "backtest/bar.h"

namespace backtest {

// Loads bars from a CSV file with header: timestamp,open,high,low,close,volume
// Throws std::runtime_error if the file cannot be opened or a row fails to parse.
std::vector<Bar> load_bars(const std::string& path);

}  // namespace backtest
