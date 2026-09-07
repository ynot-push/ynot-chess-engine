// Pawn.cpp
#include "Pawn.h"

Bitboard Pawn::PawnAttackTable[2][64];

Bitboard Pawn::attacksSlow(int square, Side side)
{
    Bitboard attacks = 0ULL;
    const int rank = square / 8;
    const int file = square % 8;

    if (side == WHITE)
    {
        if (rank + 1 < 8)
        {
            if (file - 1 >= 0) attacks |= 1ULL << ((rank + 1) * 8 + (file - 1));
            if (file + 1 < 8)  attacks |= 1ULL << ((rank + 1) * 8 + (file + 1));
        }
    }
    else // BLACK
    {
        if (rank - 1 >= 0)
        {
            if (file - 1 >= 0) attacks |= 1ULL << ((rank - 1) * 8 + (file - 1));
            if (file + 1 < 8)  attacks |= 1ULL << ((rank - 1) * 8 + (file + 1));
        }
    }

    return attacks;
}

void Pawn::initPawnAttacks()
{
    for (int square = 0; square < 64; square++)
    {
        PawnAttackTable[WHITE][square] = attacksSlow(square, WHITE);
        PawnAttackTable[BLACK][square] = attacksSlow(square, BLACK);
    }
}

Bitboard Pawn::attacks(int square, Side side)
{
    return PawnAttackTable[side][square];
}