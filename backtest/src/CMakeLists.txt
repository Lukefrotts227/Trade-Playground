find_package(nlohmann_json CONFIG REQUIRED)

add_library(backtest_engine_lib
    version.cpp
    bar_loader.cpp
    engine.cpp
    portfolio.cpp
    performance.cpp
    indicators.cpp
    rule_strategy.cpp
)

target_include_directories(backtest_engine_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(backtest_engine_lib PUBLIC
    nlohmann_json::nlohmann_json
)

add_executable(backtest_cli main.cpp)

target_link_libraries(backtest_cli PRIVATE
    backtest_engine_lib
)
