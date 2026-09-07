"""
Prints PyTorch NNUE eval (centipawns, side-to-move perspective) for a
fixed set of test FENs. Compare this output line-by-line against the
C++ verification harness's output for the same FENs — they should match
to within a few centipawns (float rounding order differs slightly
between the two implementations, so exact bit-identical isn't expected,
but anything more than ~1-2 cp off signals a real bug, not noise).

Usage:
    python verify_nnue.py nnue_best_lichess_scratch.pt
"""

import sys
import torch

from model import NNUE
from build_halfkp_chunks import encode_position, pad

TEST_FENS = [
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",              # start position
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 3 2",       # after 1.e4 e5 2.Nf3
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 5 3",    # Italian-ish, black to move
    "8/8/8/4k3/8/8/4P3/4K2R w K - 0 1",                                       # simple endgame, white to move
    "r3k2r/pp3ppp/2p1pn2/3p4/3P4/2N1PN2/PP3PPP/R3K2R w KQkq - 0 12",         # midgame, both sides can castle
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                              # sparse endgame
]


def main(pt_path):
    model = NNUE()
    state_dict = torch.load(pt_path, map_location="cpu")
    model.load_state_dict(state_dict)
    model.eval()

    print(f"loaded {pt_path}\n")

    with torch.no_grad():
        for fen in TEST_FENS:
            us_features, them_features, white_to_move = encode_position(fen)

            us_arr, _ = pad(us_features)
            them_arr, _ = pad(them_features)

            us_t = torch.from_numpy(us_arr).long().unsqueeze(0)     # (1, 32)
            them_t = torch.from_numpy(them_arr).long().unsqueeze(0)  # (1, 32)

            cp = model.predict_cp(us_t, them_t).item()

            print(f"cp={cp:8.2f}  stm={'white' if white_to_move else 'black'}  fen={fen}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: python verify_nnue.py <model.pt>")
        sys.exit(1)

    main(sys.argv[1])