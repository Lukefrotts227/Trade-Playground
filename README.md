# Trade Playground

A trading strategy playground: describe a strategy in plain English, and get a
backtest of it against historical data, explained back to you in plain English.

No finance background needed. Behind the scenes, an LLM pipeline turns your
description into a precise, structured strategy, a C++ engine backtests it, and
an LLM turns the results into a plain-English explanation.

## How it works

```
"buy when it looks oversold, sell when it looks overbought"
        |
        v
  Clarify        Rewrites your idea into a precise strategy (e.g. "RSI(14) < 30")
                  and explains any concepts it introduces along the way.
        |
        v
  Convert        Turns the clarified strategy into a structured JSON rule.
        |
        v
  Backtest       A C++ engine runs the rule against historical price data,
                  computing indicators (SMA/EMA/RSI) with proper lookback and
                  tracking every trade.
        |
        v
  Explain        Turns the numeric results (return %, trades, win rate) into a
                  plain-English summary -- honest about both wins and losses.
```

## Repo layout

- `backtest/` — C++ backtesting engine (CMake + vcpkg)
- `api/` — Python FastAPI service orchestrating the LLM pipeline (uv)

## `backtest/` (C++)

- `Bar` / `Strategy` / `Engine` / `Portfolio` — the core event-driven backtest loop.
  `Strategy::on_bar` gets full bar history up to (never past) the current bar, so
  strategies can compute indicators without any lookahead.
- `indicators.h` — SMA, EMA, RSI
- `RuleStrategy` — builds a runnable strategy directly from a JSON rule, so no
  strategy-specific C++ code is needed per strategy
- `backtest_cli` — a small CLI wrapping all of the above: given a bars CSV path and
  starting cash as arguments and a JSON rule on stdin, runs the backtest and prints
  a performance report (`total_return_pct`, `num_trades`, `win_rate_pct`) as JSON

Build/test:
```
export VCPKG_ROOT=~/vcpkg
cd backtest
cmake --preset default
cmake --build --preset default
ctest --preset default
```

### JSON rule schema

```json
{
  "name": "Simple RSI Reversal",
  "buy_when": [
    { "left": { "type": "indicator", "name": "rsi", "period": 14 },
      "operator": "lt",
      "right": { "type": "constant", "value": 30 } }
  ],
  "sell_when": [
    { "left": { "type": "indicator", "name": "rsi", "period": 14 },
      "operator": "gt",
      "right": { "type": "constant", "value": 70 } }
  ]
}
```

`buy_when`/`sell_when` are lists of conditions, implicitly AND'd. Each condition is
`left operator right`, where `left`/`right` are a `price` field (open/high/low/close/
volume), an `indicator` (`sma`/`ema`/`rsi` with a `period`), or a `constant` number.
Operators: `lt`, `lte`, `gt`, `gte`, `eq`.

## `api/` (Python)

FastAPI service, `uv`-managed. Orchestrates the pipeline above:

- `stages/clarify.py` — Clarify, via Gemini
- `stages/convert.py` — Convert, via Gemini
- `bridge.py` — invokes the compiled `backtest_cli` as a subprocess and parses its
  JSON output
- `stages/explain.py` — Explain, via Groq

All LLM calls use structured output (JSON schema / JSON mode) against Pydantic
models in `schemas/`, so every stage's output is a typed, validated Python object.

Setup:
```
cd api
uv sync
cp .env.example .env   # fill in GEMINI_API_KEY and GROQ_API_KEY
uv run uvicorn api.main:app --app-dir src
```
