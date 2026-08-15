# Trade Playground

A trading strategy playground: describe a strategy in plain English, an LLM turns it
into structured rules, a C++ engine backtests those rules against historical data,
and an LLM explains the results in plain English.

## MVP Pipeline

```
Plain English strategy
        |
        v
  [Stage 1a: Clarify]   Gemini (free tier) rewrites vague English into a precise,
        |                unambiguous strategy description, and explains any trading
        |                concepts it introduces (beginner-friendly).
        v
  [Stage 1b: Convert]   Gemini (free tier) mechanically maps the clarified text into
        |                a structured JSON rule (buy_when / sell_when conditions).
        v
  [Stage 2: Execute]    The JSON rule is evaluated by the C++ backtest engine against
        |                historical OHLCV data.
        v
  [Stage 3: Explain]    An LLM turns the backtest results into a plain-English
                         explanation. (not yet built)
```

## Repo layout

- `backtest/` — C++ backtesting engine (CMake + vcpkg)
- `api/` — Python FastAPI service orchestrating the LLM pipeline (uv)

## `backtest/` (C++)

Core pieces:
- `Bar` — one OHLCV price bar
- `Strategy` — interface a strategy implements: `on_bar(bars, current_index) -> Signal`,
  with access to full bar history (no lookahead past `current_index`)
- `Engine` — walks bars one at a time, feeding each to a `Strategy` and applying its
  signal to a `Portfolio`
- `Portfolio` — tracks cash/position, executes all-in Buy / all-out Sell at bar close,
  records a trade log
- `indicators.h` — SMA, EMA, RSI, computed with proper lookback (Wilder's smoothing for RSI)
- `RuleStrategy` — builds a `Strategy` directly from a JSON rule (via `nlohmann-json`)
  matching the schema below, so no strategy-specific C++ code is needed per strategy
- `compute_performance` — total return %, trade count, win rate from a finished run

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

- `buy_when` / `sell_when`: lists of conditions, implicitly AND'd (no OR/nesting yet)
- Each condition: `left operator right`
- `left`/`right` can be a `price` field (open/high/low/close/volume), an `indicator`
  (`sma`/`ema`/`rsi`, with a `period`), or a `constant` number
- Operators: `lt`, `lte`, `gt`, `gte`, `eq`

## `api/` (Python)

FastAPI service, `uv`-managed. LLM stages implemented so far:

- `stages/clarify.py` — Stage 1a, Gemini free tier
- `stages/convert.py` — Stage 1b, Gemini free tier

Both use Gemini's structured-output mode (`response_schema`) against Pydantic models
in `schemas/`, so responses are guaranteed to parse into typed Python objects.

Setup:
```
cd api
uv sync
cp .env.example .env   # fill in GEMINI_API_KEY and GROQ_API_KEY
uv run uvicorn api.main:app --app-dir src
```

## Not yet built

- Stage 3 (Explain results in plain English)
- A way to actually invoke the C++ engine from Python (subprocess, bindings, or CLI)
- FastAPI endpoints wiring the stages together
- OR logic / condition nesting in the rule schema
- Fees, slippage, position sizing beyond all-in/all-out
