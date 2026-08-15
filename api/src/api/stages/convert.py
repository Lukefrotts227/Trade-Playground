from google import genai
from google.genai import types

from api.schemas import StrategyRule
from api.settings import settings

_MODEL = "gemini-flash-latest"

_SYSTEM_PROMPT = """\
You convert a precise, already-clarified trading strategy description into a \
structured JSON rule.

You may only use:
- price fields: open, high, low, close, volume
- indicators: sma, ema, rsi (each requires an integer period)
- operators: lt, lte, gt, gte, eq

Map the description mechanically into buy_when and sell_when condition lists. \
Do not invent conditions, indicators, or thresholds that are not stated or clearly \
implied by the description. Do not add explanation or commentary -- only produce \
the structured rule.
"""


def convert(clarified_description: str) -> StrategyRule:
    client = genai.Client(api_key=settings.gemini_api_key)

    response = client.models.generate_content(
        model=_MODEL,
        contents=clarified_description,
        config=types.GenerateContentConfig(
            system_instruction=_SYSTEM_PROMPT,
            response_mime_type="application/json",
            response_schema=StrategyRule,
        ),
    )

    return StrategyRule.model_validate(response.parsed)
