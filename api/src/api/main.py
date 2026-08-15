from fastapi import FastAPI

app = FastAPI(title="Trade Playground API")


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}
