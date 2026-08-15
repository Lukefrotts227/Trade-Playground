from pydantic import BaseModel


class BacktestResult(BaseModel):
    total_return_pct: float
    num_trades: int
    win_rate_pct: float


class ExplainResult(BaseModel):
    explanation: str
