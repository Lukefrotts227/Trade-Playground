import json

from groq import Groq

from api.schemas import BacktestResult, ExplainResult
from api.settings import settings

_MODEL = "llama-3.3-70b-versatile"

_SYSTEM_PROMPT = """\
You are a friendly trading tutor explaining backtest results to a complete beginner.

Given the backtest's total return %, number of completed trades, and win rate %, \
write a short, plain-English explanation of how the strategy performed. Stick to \
what the numbers directly show -- do not invent reasons for the performance that \
aren't supported by the given data (e.g. don't claim "the market was volatile" \
unless that's part of the input). Keep it encouraging but honest: a bad result \
should be described clearly, not spun as good.

Respond with ONLY a JSON object of the exact form: {"explanation": "<your text>"}
No other keys, no markdown, no commentary outside the JSON object.
"""


def explain(result: BacktestResult) -> ExplainResult:
    client = Groq(api_key=settings.groq_api_key)

    response = client.chat.completions.create(
        model=_MODEL,
        messages=[
            {"role": "system", "content": _SYSTEM_PROMPT},
            {"role": "user", "content": result.model_dump_json()},
        ],
        response_format={"type": "json_object"},
    )

    content = response.choices[0].message.content
    return ExplainResult.model_validate(json.loads(content))
