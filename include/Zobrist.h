#pragma once

#include <cstdint>
#include "PieceType.h"
namespace Zobrist
{
    extern uint64_t pieceKeys[12][64];
    extern uint64_t sideKey;
    extern uint64_t castlingKeys[16];
    extern uint64_t enPassantFileKeys[8];

    void init();

    inline int castlingIndex(bool wk, bool wq, bool bk, bool bq)
    {
        return (wk ? 1 : 0) | (wq ? 2 : 0) | (bk ? 4 : 0) | (bq ? 8 : 0);
    }
}
