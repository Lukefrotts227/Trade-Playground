import logging
from pathlib import Path

from fastapi import FastAPI, Form, Request
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
from pydantic import BaseModel

from api.bridge import run_backtest
from api.datasets import get_dataset_path, list_datasets
from api.schemas import BacktestResult, ClarifyResult, StrategyRule
from api.stages.clarify import clarify
from api.stages.convert import convert
from api.stages.explain import explain

logger = logging.getLogger(__name__)

app = FastAPI(title="Trade Playground API")

_FRIENDLY_ERROR = (
    "Something went wrong generating that. This often means the free-tier LLM "
    "quota was hit -- try again in a bit."
)

_TEMPLATES_DIR = Path(__file__).resolve().parent / "templates"
templates = Jinja2Templates(directory=str(_TEMPLATES_DIR))

_DEFAULT_SYMBOL = "SPY"
_STARTING_CASH = 10000.0


class ClarifyRequest(BaseModel):
    raw_strategy_text: str


class RunRequest(BaseModel):
    clarified_description: str
    symbol: str = _DEFAULT_SYMBOL


class RunResponse(BaseModel):
    rule: StrategyRule
    result: BacktestResult
    explanation: str


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.get("/datasets")
def datasets_endpoint() -> list[str]:
    return list_datasets()


@app.post("/clarify")
def clarify_endpoint(request: ClarifyRequest) -> ClarifyResult:
    return clarify(request.raw_strategy_text)


@app.post("/run")
def run_endpoint(request: RunRequest) -> RunResponse:
    rule = convert(request.clarified_description)
    bars_path = get_dataset_path(request.symbol)
    result = run_backtest(str(bars_path), rule, _STARTING_CASH)
    explanation = explain(result)

    return RunResponse(rule=rule, result=result, explanation=explanation.explanation)


@app.get("/", response_class=HTMLResponse)
def ui_index(request: Request) -> HTMLResponse:
    return templates.TemplateResponse(request, "index.html", {"datasets": list_datasets()})


@app.post("/ui/clarify", response_class=HTMLResponse)
def ui_clarify(
    request: Request, raw_strategy_text: str = Form(...), symbol: str = Form(_DEFAULT_SYMBOL)
) -> HTMLResponse:
    try:
        result = clarify(raw_strategy_text)
    except Exception:
        logger.exception("clarify() failed")
        return templates.TemplateResponse(
            request,
            "index.html",
            {
                "raw_strategy_text": raw_strategy_text,
                "symbol": symbol,
                "datasets": list_datasets(),
                "error": _FRIENDLY_ERROR,
            },
        )

    return templates.TemplateResponse(
        request,
        "clarify_result.html",
        {
            "clarified_description": result.clarified_description,
            "explanation": result.explanation,
            "symbol": symbol,
        },
    )


@app.post("/ui/run", response_class=HTMLResponse)
def ui_run(
    request: Request, clarified_description: str = Form(...), symbol: str = Form(_DEFAULT_SYMBOL)
) -> HTMLResponse:
    try:
        rule = convert(clarified_description)
        bars_path = get_dataset_path(symbol)
        result = run_backtest(str(bars_path), rule, _STARTING_CASH)
        explanation = explain(result)
    except Exception:
        logger.exception("ui_run() failed")
        return templates.TemplateResponse(
            request,
            "index.html",
            {"datasets": list_datasets(), "error": _FRIENDLY_ERROR},
        )

    return templates.TemplateResponse(
        request,
        "run_result.html",
        {
            "symbol": symbol,
            "rule_json": rule.model_dump_json(indent=2),
            "result": result,
            "explanation": explanation.explanation,
        },
    )
