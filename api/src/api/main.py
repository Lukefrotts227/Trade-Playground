import logging
from pathlib import Path

from fastapi import FastAPI, Form, Request
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
from pydantic import BaseModel

from api.bridge import run_backtest
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

_SAMPLE_BARS_PATH = Path(__file__).resolve().parents[3] / "backtest" / "tests" / "fixtures" / "sample_bars.csv"
_STARTING_CASH = 10000.0


class ClarifyRequest(BaseModel):
    raw_strategy_text: str


class RunRequest(BaseModel):
    clarified_description: str


class RunResponse(BaseModel):
    rule: StrategyRule
    result: BacktestResult
    explanation: str


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.post("/clarify")
def clarify_endpoint(request: ClarifyRequest) -> ClarifyResult:
    return clarify(request.raw_strategy_text)


@app.post("/run")
def run_endpoint(request: RunRequest) -> RunResponse:
    rule = convert(request.clarified_description)
    result = run_backtest(str(_SAMPLE_BARS_PATH), rule, _STARTING_CASH)
    explanation = explain(result)

    return RunResponse(rule=rule, result=result, explanation=explanation.explanation)


@app.get("/", response_class=HTMLResponse)
def ui_index(request: Request) -> HTMLResponse:
    return templates.TemplateResponse(request, "index.html", {})


@app.post("/ui/clarify", response_class=HTMLResponse)
def ui_clarify(request: Request, raw_strategy_text: str = Form(...)) -> HTMLResponse:
    try:
        result = clarify(raw_strategy_text)
    except Exception:
        logger.exception("clarify() failed")
        return templates.TemplateResponse(
            request,
            "index.html",
            {"raw_strategy_text": raw_strategy_text, "error": _FRIENDLY_ERROR},
        )

    return templates.TemplateResponse(
        request,
        "clarify_result.html",
        {
            "clarified_description": result.clarified_description,
            "explanation": result.explanation,
        },
    )


@app.post("/ui/run", response_class=HTMLResponse)
def ui_run(request: Request, clarified_description: str = Form(...)) -> HTMLResponse:
    try:
        rule = convert(clarified_description)
        result = run_backtest(str(_SAMPLE_BARS_PATH), rule, _STARTING_CASH)
        explanation = explain(result)
    except Exception:
        logger.exception("ui_run() failed")
        return templates.TemplateResponse(
            request,
            "index.html",
            {"error": _FRIENDLY_ERROR},
        )

    return templates.TemplateResponse(
        request,
        "run_result.html",
        {
            "rule_json": rule.model_dump_json(indent=2),
            "result": result,
            "explanation": explanation.explanation,
        },
    )
