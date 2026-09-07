# nnue-eval-trainer
the training pipeline for my chess bot's NNUE evaluation network
## Overview

- **`build_halfkp_chunks.py`** — encodes a Lichess eval CSV (`fen,cp,depth,knodes,mate`) into HalfKP feature chunks saved as `.npz` files.
- **`model.py`** — the NNUE network
- **`train.py`** — trains the model on the generated chunks

## Requirements

- Python 3.10+
- See `requirements.txt`

