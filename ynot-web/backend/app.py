from pathlib import Path
import subprocess
from fastapi import FastAPI , HTTPException
from pydantic import BaseModel

app=FastAPI()

ROOT = Path(__file__).resolve().parents[2]
ENGINE = ROOT / "bin" / "chess_bot.exe"
NNUE = ROOT / "nnue-eval-trainer" / "nnue.bin"

class PositionRequest(BaseModel):
    fen: str

@app.get("/api/health")
def health():
    return {
        "status": "ok",
        "engine_exists": ENGINE.exists(),
        "nnue_exists": NNUE.exists(),
    }

@app.post("/api/bestmove")
def bestmove(request: PositionRequest):
    if not ENGINE.exists():
        raise HTTPException(500, "Chess engine executable not found")
    if not NNUE.exists():
        raise HTTPException(500, "NNUE weights not found")

    fen=request.fen.strip()
    if len(fen) > 200 or len(fen.split()) != 6:
        raise HTTPException(400, "Invalid FEN position")
    commands = (
        "uci\n"
        "isready\n"
        f"position fen {fen}\n"
        "go movetime 1200\n"
        "quit\n"
    )
    try:
        result = subprocess.run(
            [str(ENGINE), str(NNUE)],
            input=commands,
            capture_output=True,
            text=True,
            timeout=15,
            cwd=str(ROOT),
        )
    except subprocess.TimeoutExpired:
        raise HTTPException(504, "Chess engine search timed out")
    except OSError as error:
        raise HTTPException(500, f"Could not launch engine: {error}")
    if result.returncode != 0:
        raise HTTPException(500, "Chess engine failed to run")
    
    for line in result.stdout.splitlines():
        if line.startswith("bestmove "):
            move = line.split()[1]

            if move == "(none)":
                raise HTTPException(400, "No legal move available")

            return {"move": move}

    raise HTTPException(
        500,
        "Engine did not return a bestmove. Check its UCI output."
    )
