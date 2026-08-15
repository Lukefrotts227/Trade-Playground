from pydantic import BaseModel


class ClarifyResult(BaseModel):
    clarified_description: str
    explanation: str
