#include <gtest/gtest.h>

#include "backtest/version.h"

TEST(Version, IsNotEmpty) {
  EXPECT_STRNE(backtest::kVersion, "");
}
