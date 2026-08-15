from pathlib import Path

_DATA_DIR = Path(__file__).resolve().parents[3] / "backtest" / "data"

# Add a new dataset by dropping a CSV (timestamp,open,high,low,close,volume) into
# backtest/data/ and adding one line here.
DATASETS: dict[str, Path] = {
    "SPY": _DATA_DIR / "spy.csv",
}


def list_datasets() -> list[str]:
    return sorted(DATASETS.keys())


def get_dataset_path(symbol: str) -> Path:
    try:
        return DATASETS[symbol]
    except KeyError:
        raise ValueError(f"Unknown dataset: {symbol!r}. Available: {list_datasets()}") from None
