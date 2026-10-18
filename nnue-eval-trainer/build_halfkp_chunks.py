"""FEN -> HalfKP NPZ chunk encoder for lichess_dataset.csv.

Expected CSV schema:
    fen,cp,depth,knodes,mate

Output NPZ fields:
    us       : (N, 32) int32
    them     : (N, 32) int32
    num_us   : (N,) int8
    num_them : (N,) int8
    target   : (N,) float32
    mate     : (N,) int16
    source   : (N,) int64
"""

import csv
import glob
import os

import chess
import numpy as np

# Configuration
INPUT_CSV_GLOB = "lichess_dataset.csv"
OUTPUT_DIR = "./lichess_chunks"
ROWS_PER_CHUNK = 200_000
MATE_CP_VALUE = 1000.0
SOURCE_TAG = 1

# HalfKP Configuration
NUM_SQUARES = 64
NUM_PIECE_TYPES = 10  
NUM_KING_SQUARES = 64
NUM_FEATURES = NUM_SQUARES * NUM_PIECE_TYPES * NUM_KING_SQUARES 

MAX_FEATURES = 32
PAD_VALUE = -1


def orient(is_white_pov: bool, square: int) -> int:

    return square if is_white_pov else square ^ 0x38


def piece_index(
    piece_type_0based: int, piece_is_white: bool, is_white_pov: bool
) -> int:

    is_ours = piece_is_white == is_white_pov
    return piece_type_0based if is_ours else piece_type_0based + 5


def halfkp_features(board: chess.Board, is_white_pov: bool) -> list[int]:

    king_square = board.king(is_white_pov)
    if king_square is None:
        raise ValueError("Position has no king for the requested side")

    king_oriented = orient(is_white_pov, king_square)
    features = []

    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece is None or piece.piece_type == chess.KING:
            continue

        p_idx = piece_index(
            piece_type_0based=piece.piece_type - 1,
            piece_is_white=(piece.color == chess.WHITE),
            is_white_pov=is_white_pov,
        )
        sq_oriented = orient(is_white_pov, square)
        idx = sq_oriented + p_idx * 64 + king_oriented * 640

        if not (0 <= idx < NUM_FEATURES):
            raise ValueError(f"HalfKP index out of range: {idx}")

        features.append(idx)

    return features


def encode_position(fen: str) -> tuple[list[int], list[int], bool]:

    board = chess.Board(fen)
    white_to_move = board.turn == chess.WHITE

    white_features = halfkp_features(board, is_white_pov=True)
    black_features = halfkp_features(board, is_white_pov=False)

    if white_to_move:
        return white_features, black_features, True
    return black_features, white_features, False


def parse_target(cp_str: str, mate_str: str) -> tuple[float, int]:

    cp_str = (cp_str or "").strip()
    mate_str = (mate_str or "").strip()

    if mate_str:
        mate = int(mate_str)
        if mate == 0:
            raise ValueError("Mate value cannot be zero")
        return (MATE_CP_VALUE if mate > 0 else -MATE_CP_VALUE), mate

    if cp_str:
        cp = float(cp_str)
        if not np.isfinite(cp):
            raise ValueError("CP is not finite")
        return max(-MATE_CP_VALUE, min(MATE_CP_VALUE, cp)), 0

    raise ValueError("Row contains neither CP nor mate evaluation")


def pad(features: list[int]) -> tuple[np.ndarray, int]:

    n = len(features)
    if n > MAX_FEATURES:
        raise ValueError(f"Too many features: {n} > {MAX_FEATURES}")

    arr = np.full(MAX_FEATURES, PAD_VALUE, dtype=np.int32)
    if n:
        arr[:n] = features
    return arr, n


def verify_encoder() -> None:

    assert NUM_FEATURES == 40960

    try:
        import model as _model

        assert (
            _model.NUM_FEATURES == NUM_FEATURES
        ), f"Mismatch: script={NUM_FEATURES}, model.py={_model.NUM_FEATURES}"
    except ImportError:
        pass

    start_fen = (
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    )
    us, them, white_to_move = encode_position(start_fen)
    assert white_to_move is True
    assert len(us) == 30 and len(them) == 30
    assert len(us) == len(set(us)) and len(them) == len(set(them))

    black_turn_fen = (
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
    )
    us2, them2, white_to_move2 = encode_position(black_turn_fen)
    assert white_to_move2 is False
    assert sorted(us2) == sorted(them) and sorted(them2) == sorted(us)

    assert parse_target("250", "") == (250.0, 0)
    assert parse_target("", "15") == (1000.0, 15)
    assert parse_target("", "-7") == (-1000.0, -7)
    assert parse_target("5000", "") == (1000.0, 0)

    arr, n = pad([1, 2, 3])
    assert n == 3 and np.array_equal(arr[:3], [1, 2, 3])
    assert np.all(arr[3:] == PAD_VALUE)


def process_file(csv_path: str, chunk_start_idx: int) -> int:

    us_buf, them_buf = [], []
    num_us_buf, num_them_buf = [], []
    target_buf, mate_buf, source_buf = [], [], []

    chunk_idx = chunk_start_idx
    rows_in_chunk = 0
    total_rows = errors = cp_rows = mate_rows = 0

    def flush() -> None:

        nonlocal chunk_idx, rows_in_chunk
        if rows_in_chunk == 0:
            return

        out_path = os.path.join(
            OUTPUT_DIR, f"lichess_chunk_{chunk_idx:04d}.npz"
        )
        np.savez(
            out_path,
            us=np.asarray(us_buf, dtype=np.int32),
            them=np.asarray(them_buf, dtype=np.int32),
            num_us=np.asarray(num_us_buf, dtype=np.int8),
            num_them=np.asarray(num_them_buf, dtype=np.int8),
            target=np.asarray(target_buf, dtype=np.float32),
            mate=np.asarray(mate_buf, dtype=np.int16),
            source=np.asarray(source_buf, dtype=np.int64),
        )

        print(f"  Wrote {out_path} ({rows_in_chunk:,} rows)")
        chunk_idx += 1

        for buf in (
            us_buf,
            them_buf,
            num_us_buf,
            num_them_buf,
            target_buf,
            mate_buf,
            source_buf,
        ):
            buf.clear()
        rows_in_chunk = 0

    with open(csv_path, "r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        expected_columns = {"fen", "cp", "depth", "knodes", "mate"}
        actual_columns = set(reader.fieldnames or [])

        if missing := (expected_columns - actual_columns):
            raise RuntimeError(f"CSV missing columns: {missing}")

        for row in reader:
            total_rows += 1
            fen = (row["fen"] or "").strip()
            if not fen:
                errors += 1
                continue

            try:
                target_white_pov, mate_distance = parse_target(
                    row["cp"], row["mate"]
                )
                us_features, them_features, white_to_move = encode_position(fen)

                target_cp = (
                    target_white_pov if white_to_move else -target_white_pov
                )
                target_cp = max(-MATE_CP_VALUE, min(MATE_CP_VALUE, target_cp))

                us_arr, n_us = pad(us_features)
                them_arr, n_them = pad(them_features)
            except Exception:
                errors += 1
                continue

            if mate_distance != 0:
                mate_rows += 1
            else:
                cp_rows += 1

            us_buf.append(us_arr)
            them_buf.append(them_arr)
            num_us_buf.append(n_us)
            num_them_buf.append(n_them)
            target_buf.append(target_cp)
            mate_buf.append(mate_distance)
            source_buf.append(SOURCE_TAG)

            rows_in_chunk += 1
            if rows_in_chunk >= ROWS_PER_CHUNK:
                flush()

            if total_rows % 500_000 == 0:
                processed = cp_rows + mate_rows
                print(
                    f"  {total_rows:,} scanned | {processed:,} encoded | "
                    f"{cp_rows:,} CP | {mate_rows:,} mate | {errors:,} errors"
                )

    flush()

    print(
        f"\nFinished {csv_path}\n"
        f"  Total: {total_rows:,} | CP: {cp_rows:,} | "
        f"Mate: {mate_rows:,} | Errors: {errors:,}"
    )

    if total_rows and (error_rate := (errors / total_rows) * 100) > 1.0:
        print(f"WARNING: Error rate is {error_rate:.2f}% (> 1%)")

    return chunk_idx


if __name__ == "__main__":
    verify_encoder()

    files = sorted(glob.glob(INPUT_CSV_GLOB))
    if not files:
        raise FileNotFoundError(f"No files matched: {INPUT_CSV_GLOB}")

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    next_chunk_idx = 0
    for path in files:
        print(f"\nProcessing: {path}")
        next_chunk_idx = process_file(path, next_chunk_idx)

    print(
        f"\nEncoding Complete. {next_chunk_idx} chunks generated in {OUTPUT_DIR}"
    )