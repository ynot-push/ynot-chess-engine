import glob
import json
import os
import random
import time
import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import Dataset, DataLoader

from model import (
    NNUE, PAD_IDX, K, CP_LOSS_WEIGHT, MATE_CP_VALUE,
    NUM_FEATURES, FT_DIM, L1, L2,
    target_to_winprob, target_to_scaled_cp,
)

CHUNK_DIR = "./lichess_chunks"
BATCH_SIZE = 8192
TRAIN_FROM_SCRATCH = True
EPOCHS = 40
LR = 2e-4
LR_STEP_SIZE = 8
LR_GAMMA = 0.5

VAL_CHUNKS = 15
TEST_CHUNKS = 15

RESUME_PATH = "nnue_best.pt"

_RUN_TAG = "scratch" if TRAIN_FROM_SCRATCH else "phase3"
CHECKPOINT_PATH = f"checkpoint_{_RUN_TAG}.pt"
BEST_PATH = f"nnue_best_{_RUN_TAG}.pt"
FINAL_PATH = f"nnue_final_{_RUN_TAG}.pt"
RUN_CONFIG_JSON_PATH = f"run_config_{_RUN_TAG}.json"

BUCKET_EDGES = [100, 300, 600] 
VAL_SPLIT_SEED = 1234


class NNUEChunkDataset(Dataset):

    def __init__(self, npz_path):
        data = np.load(npz_path, mmap_mode="r")
        self.us = data["us"]
        self.them = data["them"]
        self.target = data["target"]
        self.n = self.target.shape[0]

    def __len__(self):
        return self.n

    def __getitem__(self, i):
        us = torch.from_numpy(np.asarray(self.us[i], dtype=np.int64))
        them = torch.from_numpy(np.asarray(self.them[i], dtype=np.int64))
        target = torch.tensor(float(self.target[i]), dtype=torch.float32)
        return us, them, target


class ChunkShuffleLoader:

    def __init__(self, chunk_paths, batch_size, shuffle_chunks=True):
        self.chunk_paths = list(chunk_paths)
        self.batch_size = batch_size
        self.shuffle_chunks = shuffle_chunks

    def __iter__(self):
        paths = self.chunk_paths.copy()
        if self.shuffle_chunks:
            np.random.shuffle(paths)

        for path in paths:
            data = np.load(path)
            us = torch.from_numpy(data["us"].astype(np.int64))
            them = torch.from_numpy(data["them"].astype(np.int64))
            target = torch.from_numpy(data["target"].astype(np.float32))

            n = target.shape[0]

            if "mate" in data.files:
                mate = torch.from_numpy(data["mate"].astype(np.int64))
            else:
                mate = torch.zeros(n, dtype=torch.int64)

            perm = torch.randperm(n)
            us = us.index_select(0, perm)
            them = them.index_select(0, perm)
            target = target.index_select(0, perm)
            mate = mate.index_select(0, perm)

            for start in range(0, n, self.batch_size):
                end = min(start + self.batch_size, n)
                yield (
                    us[start:end],
                    them[start:end],
                    target[start:end],
                    mate[start:end],
                )

    def __len__(self):
        return None


def build_datasets(chunk_dir):
    chunk_paths = sorted(glob.glob(f"{chunk_dir}/*.npz"))
    assert chunk_paths, f"no .npz files found in {chunk_dir}"
    assert len(chunk_paths) > VAL_CHUNKS + TEST_CHUNKS, (
        f"only {len(chunk_paths)} chunks found, need more than "
        f"VAL_CHUNKS+TEST_CHUNKS={VAL_CHUNKS + TEST_CHUNKS} to leave any for training"
    )

    rng = random.Random(VAL_SPLIT_SEED)
    rng.shuffle(chunk_paths)

    test_paths = sorted(chunk_paths[:TEST_CHUNKS])
    val_paths = sorted(chunk_paths[TEST_CHUNKS:TEST_CHUNKS + VAL_CHUNKS])
    train_paths = sorted(chunk_paths[TEST_CHUNKS + VAL_CHUNKS:])

    print(f"chunks: {len(train_paths)} train / {len(val_paths)} val / "
          f"{len(test_paths)} test (reserved, not used this run)")
    return train_paths, val_paths, test_paths


def bucket_stats(pred_cp, target_cp):
    abs_err = (pred_cp - target_cp).abs()
    abs_target = target_cp.abs()
    edges = [0.0] + BUCKET_EDGES + [float("inf")]
    labels = ["close", "mid", "decided", "blowout"]
    out = {}
    for lo, hi, label in zip(edges[:-1], edges[1:], labels):
        mask = (abs_target >= lo) & (abs_target < hi)
        n = mask.sum().item()
        mae = abs_err[mask].mean().item() if n > 0 else float("nan")
        out[label] = (mae, n)
    return out


class MetricsAccumulator:

    GROUPS = ["overall", "cp_only", "mate_only", "close", "mid", "decided", "blowout"]

    def __init__(self, bucket_edges):
        self.edges = [0.0] + list(bucket_edges) + [float("inf")]
        self.bucket_labels = ["close", "mid", "decided", "blowout"]
        self._reset()

    def _reset(self):
        self.n = {g: 0 for g in self.GROUPS}
        self.sum_abs_err = {g: 0.0 for g in self.GROUPS}
        self.sum_sq_err = {g: 0.0 for g in self.GROUPS}
        self.sum_signed_err = {g: 0.0 for g in self.GROUPS}
        self.sign_correct = {g: 0 for g in self.GROUPS}

    def _accumulate(self, group, mask, err, sq_err, sign_ok):
        cnt = mask.sum().item()
        if cnt == 0:
            return
        self.n[group] += cnt
        self.sum_abs_err[group] += err[mask].abs().sum().item()
        self.sum_sq_err[group] += sq_err[mask].sum().item()
        self.sum_signed_err[group] += err[mask].sum().item()
        self.sign_correct[group] += sign_ok[mask].sum().item()

    def update(self, pred_cp, target_cp, is_mate):
        err = pred_cp - target_cp
        sq_err = err * err
        sign_ok = (pred_cp > 0) == (target_cp > 0)

        all_mask = torch.ones_like(target_cp, dtype=torch.bool)
        self._accumulate("overall", all_mask, err, sq_err, sign_ok)
        self._accumulate("cp_only", ~is_mate, err, sq_err, sign_ok)
        self._accumulate("mate_only", is_mate, err, sq_err, sign_ok)

        abs_target = target_cp.abs()
        for lo, hi, label in zip(self.edges[:-1], self.edges[1:], self.bucket_labels):
            mask = (abs_target >= lo) & (abs_target < hi)
            self._accumulate(label, mask, err, sq_err, sign_ok)

    def summary(self):
        out = {}
        for g in self.GROUPS:
            n = self.n[g]
            if n == 0:
                out[g] = None
                continue
            out[g] = {
                "n": n,
                "mae": self.sum_abs_err[g] / n,
                "rmse": (self.sum_sq_err[g] / n) ** 0.5,
                "bias": self.sum_signed_err[g] / n,
                "sign_acc": self.sign_correct[g] / n,
            }
        return out

    def format_lines(self):
        s = self.summary()
        lines = []
        for g in self.GROUPS:
            st = s[g]
            if st is None:
                lines.append(f"    {g:>9}: n/a")
            else:
                lines.append(
                    f"    {g:>9}(n={st['n']:>10,}): MAE={st['mae']:7.1f}cp  "
                    f"RMSE={st['rmse']:7.1f}cp  bias={st['bias']:+7.1f}cp  "
                    f"sign={st['sign_acc']*100:5.1f}%"
                )
        return "\n".join(lines)


def run_epoch(model, loader, optimizer, device, train=True, epoch=0, log_every=100):
    model.train(train)
    total_loss = 0.0
    total_bce = 0.0
    total_huber = 0.0
    total_n = 0
    bce_fn = nn.BCEWithLogitsLoss()
    huber_fn = nn.SmoothL1Loss()

    phase = "train" if train else "val"
    epoch_start = time.time()
    window_start = epoch_start
    window_loss_sum = 0.0
    window_correct = 0
    window_n = 0

    metrics = MetricsAccumulator(BUCKET_EDGES)

    for batch_idx, (us, them, target_cp, mate) in enumerate(loader, start=1):
        us = us.to(device, non_blocking=True)
        them = them.to(device, non_blocking=True)
        target_cp = target_cp.to(device, non_blocking=True)
        mate = mate.to(device, non_blocking=True)
        is_mate = mate != 0

        target_wp = target_to_winprob(target_cp, k=K)
        target_scaled = target_to_scaled_cp(target_cp, k=K, mate_cp=MATE_CP_VALUE)

        with torch.set_grad_enabled(train):
            logits = model(us, them)
            loss_prob = bce_fn(logits, target_wp)
            loss_cp = huber_fn(logits, target_scaled)
            loss = loss_prob + CP_LOSS_WEIGHT * loss_cp

            if train:
                optimizer.zero_grad()
                loss.backward()
                optimizer.step()

        bs = target_cp.shape[0]
        total_loss += loss.item() * bs
        total_bce += loss_prob.item() * bs
        total_huber += loss_cp.item() * bs
        total_n += bs

        with torch.no_grad():
            pred_wp = torch.sigmoid(logits)
            correct = ((pred_wp > 0.5) == (target_wp > 0.5)).sum().item()
            pred_cp = logits.detach() * K
            metrics.update(pred_cp, target_cp, is_mate)

        window_loss_sum += loss.item() * bs
        window_correct += correct
        window_n += bs

        if batch_idx % log_every == 0:
            elapsed = time.time() - window_start
            pos_per_sec = window_n / elapsed if elapsed > 0 else 0.0
            avg_loss = window_loss_sum / window_n
            acc = window_correct / window_n
            total_elapsed = time.time() - epoch_start
            print(
                f"  [{phase}] epoch {epoch:3d} | batch {batch_idx:6d} | "
                f"loss {avg_loss:.5f} | acc {acc*100:5.2f}% | "
                f"{pos_per_sec:8.0f} pos/s | elapsed {total_elapsed:7.1f}s"
            )
            window_start = time.time()
            window_loss_sum = 0.0
            window_correct = 0
            window_n = 0

    print(f"  [{phase}] epoch {epoch:3d} metrics:")
    print(metrics.format_lines())

    return {
        "loss": total_loss / total_n,
        "bce": total_bce / total_n,
        "huber": total_huber / total_n,
        "metrics": metrics.summary(),
    }


def build_run_config():
    return {
        "K": K,
        "MATE_CP_VALUE": MATE_CP_VALUE,
        "CP_LOSS_WEIGHT": CP_LOSS_WEIGHT,
        "NUM_FEATURES": NUM_FEATURES,
        "FT_DIM": FT_DIM,
        "L1": L1,
        "L2": L2,
        "BATCH_SIZE": BATCH_SIZE,
        "EPOCHS": EPOCHS,
        "LR": LR,
        "LR_STEP_SIZE": LR_STEP_SIZE,
        "LR_GAMMA": LR_GAMMA,
        "VAL_CHUNKS": VAL_CHUNKS,
        "TEST_CHUNKS": TEST_CHUNKS,
        "VAL_SPLIT_SEED": VAL_SPLIT_SEED,
        "CHUNK_DIR": CHUNK_DIR,
        "TRAIN_FROM_SCRATCH": TRAIN_FROM_SCRATCH,
        "RUN_TAG": _RUN_TAG,
        "BUCKET_EDGES": BUCKET_EDGES,
    }


def main():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("device:", device)

    train_paths, val_paths, test_paths = build_datasets(CHUNK_DIR)

    run_config = build_run_config()
    with open(RUN_CONFIG_JSON_PATH, "w") as f:
        json.dump({
            "config": run_config,
            "train_paths": train_paths,
            "val_paths": val_paths,
            "test_paths": test_paths,
        }, f, indent=2)
    print(f"wrote run config + chunk splits to {RUN_CONFIG_JSON_PATH}")

    train_loader = ChunkShuffleLoader(train_paths, BATCH_SIZE, shuffle_chunks=True)
    val_loader = ChunkShuffleLoader(val_paths, BATCH_SIZE, shuffle_chunks=False)

    model = NNUE().to(device)
    optimizer = torch.optim.Adam(model.parameters(), lr=LR)
    scheduler = torch.optim.lr_scheduler.StepLR(optimizer, step_size=LR_STEP_SIZE, gamma=LR_GAMMA)

    start_epoch = 1
    best_val = float("inf")

    if os.path.exists(CHECKPOINT_PATH):
        ckpt = torch.load(CHECKPOINT_PATH, map_location=device)
        model.load_state_dict(ckpt["model"])
        optimizer.load_state_dict(ckpt["optimizer"])
        scheduler.load_state_dict(ckpt["scheduler"])
        start_epoch = ckpt["epoch"] + 1
        best_val = ckpt["best_val"]
        print(f"resumed from {CHECKPOINT_PATH}: starting at epoch {start_epoch}, "
              f"best_val so far = {best_val:.5f}")
    elif TRAIN_FROM_SCRATCH:
        print(f"TRAIN_FROM_SCRATCH=True - random init, ignoring {RESUME_PATH}; "
              f"training {EPOCHS} epochs at LR={LR} (step every {LR_STEP_SIZE} "
              f"epochs, gamma={LR_GAMMA})")
    elif RESUME_PATH is not None and os.path.exists(RESUME_PATH):
        model.load_state_dict(torch.load(RESUME_PATH, map_location=device))
        print(f"loaded weights from {RESUME_PATH} - optimizer/scheduler reset to "
              f"fresh LR={LR}; best_val starts at +inf since the loss function "
              f"changed and the old best_val isn't on the same scale")
    else:
        print("no checkpoint found - starting fresh")

    for epoch in range(start_epoch, EPOCHS + 1):
        epoch_start = time.time()
        train_result = run_epoch(model, train_loader, optimizer, device, train=True, epoch=epoch)
        val_result = run_epoch(model, val_loader, optimizer, device, train=False, epoch=epoch)

        current_lr = optimizer.param_groups[0]["lr"]
        scheduler.step()
        epoch_time = time.time() - epoch_start

        train_loss, val_loss = train_result["loss"], val_result["loss"]
        print(
            f"epoch {epoch:3d} | LR={current_lr:.2e} | "
            f"train_loss {train_loss:.5f} (bce={train_result['bce']:.5f} "
            f"huber={train_result['huber']:.5f}) | "
            f"val_loss {val_loss:.5f} (bce={val_result['bce']:.5f} "
            f"huber={val_result['huber']:.5f}) | "
            f"time {epoch_time:7.1f}s"
        )

        if val_loss < best_val:
            best_val = val_loss
            torch.save(model.state_dict(), BEST_PATH)
            print(f"  -> saved new best ({BEST_PATH}, val_loss={val_loss:.5f})")

        torch.save({
            "model": model.state_dict(),
            "optimizer": optimizer.state_dict(),
            "scheduler": scheduler.state_dict(),
            "epoch": epoch,
            "best_val": best_val,
            "config": run_config,
            "train_paths": train_paths,
            "val_paths": val_paths,
            "test_paths": test_paths,
        }, CHECKPOINT_PATH)

    torch.save(model.state_dict(), FINAL_PATH)


if __name__ == "__main__":
    main()