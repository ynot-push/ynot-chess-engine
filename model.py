import torch
import torch.nn as nn

NUM_FEATURES = 40960
FT_DIM = 256
L1 = 32
L2 = 32
PAD_IDX = -1
K = 600.0
MATE_CP_VALUE = 1000.0
CP_LOSS_WEIGHT = 1.0


class ClippedReLU(nn.Module):

    def forward(self, x):
        return torch.clamp(x, 0.0, 1.0)

class NNUE(nn.Module):

    def __init__(
        self,
        num_features=NUM_FEATURES,
        ft_dim=FT_DIM,
        l1=L1,
        l2=L2,
    ):
        super().__init__()
        self._num_features = num_features

        self.feature_transformer = nn.EmbeddingBag(
            num_embeddings=num_features + 1,
            embedding_dim=ft_dim,
            mode="sum",
            padding_idx=num_features,
        )

        self.l1= nn.Linear(ft_dim * 2, l1)
        self.l2= nn.Linear(l1, l2)
        self.output = nn.Linear(l2, 1)
        self.crelu = ClippedReLU()

    def _encode_side(self, idx):
        if idx.dtype != torch.long:
            idx= idx.long()

        safe_idx = torch.where(
            idx < 0,
            torch.full_like(idx, self._num_features),
            idx,
        )
        if torch.any(safe_idx > self._num_features):
            raise ValueError(
                f"Feature index out of range. "
                f"Valid range: 0..{self._num_features - 1}, "
                f"padding={self._num_features}"
            )
        return self.feature_transformer(safe_idx)

    def forward(self, us_idx, them_idx):
        us = self._encode_side(us_idx)
        them = self._encode_side(them_idx)

        x = torch.cat([us, them], dim=1)
        x = self.crelu(x)
        x = self.l1(x)
        x = self.crelu(x)
        x = self.l2(x)
        x = self.crelu(x)
        x = self.output(x)

        return x.squeeze(-1)

    def predict_winprob(self, us_idx, them_idx):
        return torch.sigmoid(
            self.forward(us_idx, them_idx)
        )

    def predict_cp(self, us_idx, them_idx):
        return self.forward(us_idx, them_idx) * K


def target_to_winprob(target_cp, k=K):
    return torch.sigmoid(target_cp / k)


def target_to_scaled_cp(
    target_cp,
    k=K,
    mate_cp=MATE_CP_VALUE,
):
    clipped = torch.clamp(
        target_cp,
        -mate_cp,
        mate_cp,
    )

    return clipped / k


def mate_to_cp(mate, mate_cp=MATE_CP_VALUE):
    if mate > 0:
        return float(mate_cp)

    if mate < 0:
        return float(-mate_cp)

    return 0.0