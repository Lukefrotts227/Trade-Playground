from typing import Literal, Union

from pydantic import BaseModel, Field


class PriceRef(BaseModel):
    type: Literal["price"] = "price"
    field: Literal["open", "high", "low", "close", "volume"]


class IndicatorRef(BaseModel):
    type: Literal["indicator"] = "indicator"
    name: Literal["sma", "ema", "rsi"]
    period: int = Field(gt=0)


class ConstantRef(BaseModel):
    type: Literal["constant"] = "constant"
    value: float


ValueRef = Union[PriceRef, IndicatorRef, ConstantRef]


class Condition(BaseModel):
    left: ValueRef = Field(discriminator="type")
    operator: Literal["lt", "lte", "gt", "gte", "eq"]
    right: ValueRef = Field(discriminator="type")


class StrategyRule(BaseModel):
    name: str
    buy_when: list[Condition]
    sell_when: list[Condition]
