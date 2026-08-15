from typing import Literal, Union

from pydantic import BaseModel


class PriceRef(BaseModel):
    type: Literal["price"] = "price"
    field: Literal["open", "high", "low", "close", "volume"]


class IndicatorRef(BaseModel):
    type: Literal["indicator"] = "indicator"
    name: Literal["sma", "ema", "rsi"]
    # Not constrained with Field(gt=0): Gemini's structured-output schema format
    # does not support exclusiveMinimum. Positivity is enforced by prompt instruction.
    period: int


class ConstantRef(BaseModel):
    type: Literal["constant"] = "constant"
    value: float


ValueRef = Union[PriceRef, IndicatorRef, ConstantRef]


class Condition(BaseModel):
    # No discriminator here: the Gemini structured-output schema format does not
    # support Pydantic's oneOf/discriminator construct. Pydantic's default "smart"
    # union matching still correctly resolves the right ValueRef subtype via the
    # literal "type" field at validation time.
    left: ValueRef
    operator: Literal["lt", "lte", "gt", "gte", "eq"]
    right: ValueRef


class StrategyRule(BaseModel):
    name: str
    buy_when: list[Condition]
    sell_when: list[Condition]
