find_package(GTest CONFIG REQUIRED)

add_executable(backtest_engine_tests
    version_test.cpp
    bar_loader_test.cpp
    engine_test.cpp
    portfolio_test.cpp
    performance_test.cpp
    indicators_test.cpp
    rule_strategy_test.cpp
)

target_link_libraries(backtest_engine_tests PRIVATE
    backtest_engine_lib
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(backtest_engine_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
