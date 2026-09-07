"""
Export NNUE weights from a PyTorch state_dict (.pt) into a flat binary
file the C++ engine reads with plain fread() — no LibTorch at runtime.

Usage:
    python export_nnue_weights.py nnue_best_lichess_scratch.pt nnue.bin
"""

import sys
import struct
import torch

from model import NUM_FEATURES, FT_DIM, L1, L2


def export_weights(pt_path, out_path):
    state_dict = torch.load(pt_path, map_location="cpu")

    def get(name):
        return state_dict[name].detach().cpu().contiguous()

    ft_weight  = get("feature_transformer.weight")  # (40961, 256)
    l1_weight  = get("l1.weight")                    # (32, 512)
    l1_bias    = get("l1.bias")                        # (32,)
    l2_weight  = get("l2.weight")                       # (32, 32)
    l2_bias    = get("l2.bias")                           # (32,)
    out_weight = get("output.weight")                       # (1, 32)
    out_bias   = get("output.bias")                           # (1,)

    assert ft_weight.shape  == (NUM_FEATURES + 1, FT_DIM)
    assert l1_weight.shape  == (L1, FT_DIM * 2)
    assert l1_bias.shape    == (L1,)
    assert l2_weight.shape  == (L2, L1)
    assert l2_bias.shape    == (L2,)
    assert out_weight.shape == (1, L2)
    assert out_bias.shape   == (1,)

    with open(out_path, "wb") as f:
        f.write(b"NNUE")
        f.write(struct.pack("<iiii", NUM_FEATURES, FT_DIM, L1, L2))

        for t in [ft_weight, l1_weight, l1_bias, l2_weight, l2_bias, out_weight, out_bias]:
            f.write(t.numpy().astype("<f4").tobytes())

    print(f"wrote {out_path}")
    print(f"  feature_transformer.weight: {tuple(ft_weight.shape)}")
    print(f"  l1: {tuple(l1_weight.shape)} / {tuple(l1_bias.shape)}")
    print(f"  l2: {tuple(l2_weight.shape)} / {tuple(l2_bias.shape)}")
    print(f"  output: {tuple(out_weight.shape)} / {tuple(out_bias.shape)}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("usage: python export_nnue_weights.py <model.pt> <out.bin>")
        sys.exit(1)

    export_weights(sys.argv[1], sys.argv[2])