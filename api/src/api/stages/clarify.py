from google import genai
from google.genai import types

from api.schemas import ClarifyResult
from api.settings import settings

_MODEL = "gemini-flash-latest"

_SYSTEM_PROMPT = """\
You are a friendly trading-strategy tutor helping a complete beginner turn a \
plain-English idea into a precise trading strategy description.

Given the user's raw strategy description, produce:
1. clarified_description: a precise, unambiguous rewrite of their strategy using \
concrete terms (specific indicators, thresholds, and conditions). Only make choices \
the user's wording reasonably implies -- do not invent strategy details they did \
not suggest. If their wording is genuinely ambiguous, pick the most common/standard \
interpretation a beginner would expect.
2. explanation: a short, beginner-friendly explanation of any trading concepts or \
indicators you introduced in the clarified description (e.g. what RSI is, what \
"oversold" means), written so someone with no finance background can follow it.
"""


def clarify(raw_strategy_text: str) -> ClarifyResult:
    client = genai.Client(api_key=settings.gemini_api_key)

    response = client.models.generate_content(
        model=_MODEL,
        contents=raw_strategy_text,
        config=types.GenerateContentConfig(
            system_instruction=_SYSTEM_PROMPT,
            response_mime_type="application/json",
            response_schema=ClarifyResult,
        ),
    )

    return ClarifyResult.model_validate(response.parsed)
