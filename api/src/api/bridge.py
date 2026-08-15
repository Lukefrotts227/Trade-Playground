import json
import os
import subprocess
from pathlib import Path

from api.schemas import BacktestResult, StrategyRule

_DEFAULT_CLI_PATH = Path(__file__).resolve().parents[3] / "backtest" / "build" / "src" / "backtest_cli"


def _cli_path() -> Path:
    override = os.environ.get("BACKTEST_CLI_PATH")
    return Path(override) if override else _DEFAULT_CLI_PATH


def run_backtest(bars_csv_path: str, rule: StrategyRule, starting_cash: float) -> BacktestResult:
    cli_path = _cli_path()
    if not cli_path.exists():
        raise FileNotFoundError(
            f"backtest_cli not found at {cli_path}. Build it first "
            "(cmake --build --preset default in backtest/), or set BACKTEST_CLI_PATH."
        )

    process = subprocess.run(
        [str(cli_path), bars_csv_path, str(starting_cash)],
        input=rule.model_dump_json(),
        capture_output=True,
        text=True,
    )

    if process.returncode != 0:
        raise RuntimeError(f"backtest_cli failed: {process.stderr.strip()}")

    return BacktestResult.model_validate(json.loads(process.stdout))
