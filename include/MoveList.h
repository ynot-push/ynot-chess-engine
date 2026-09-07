#pragma once

#include "Move.h"

constexpr int MAX_MOVES = 256;

struct MoveList
{
    Move moves[MAX_MOVES];
    int count = 0;

    inline void push(int from, int to, Piece piece, Piece captured, int promotion, bool isEP, bool isCastling)
    {
        moves[count++] = Move{ from, to, piece, captured, promotion, isEP, isCastling };
    }

    inline void push(const Move& m) { moves[count++] = m; }

    inline Move*       begin()       { return moves; }
    inline Move*       end()         { return moves + count; }
    inline const Move* begin() const { return moves; }
    inline const Move* end()   const { return moves + count; }

    inline int  size()  const { return count; }
    inline bool empty() const { return count == 0; }

    inline Move&       operator[](int i)       { return moves[i]; }
    inline const Move& operator[](int i) const { return moves[i]; }
};
