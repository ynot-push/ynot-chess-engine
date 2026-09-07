#include "Knight.h"

Bitboard Knight::KnightAttackTable[64];

Bitboard Knight::attacksSlow(int square)
{
    Bitboard attacks = 0ULL;
    const int rank = square / 8;
    const int file = square % 8;

    if (rank + 2 < 8 && file + 1 < 8) attacks |= 1ULL << ((rank + 2) * 8 + (file + 1));
    if (rank + 2 < 8 && file - 1 >= 0) attacks |= 1ULL << ((rank + 2) * 8 + (file - 1));
    if (rank - 2 >= 0 && file + 1 < 8) attacks |= 1ULL << ((rank - 2) * 8 + (file + 1));
    if (rank - 2 >= 0 && file - 1 >= 0) attacks |= 1ULL << ((rank - 2) * 8 + (file - 1));
    if (rank + 1 < 8 && file + 2 < 8) attacks |= 1ULL << ((rank + 1) * 8 + (file + 2));
    if (rank + 1 < 8 && file - 2 >= 0) attacks |= 1ULL << ((rank + 1) * 8 + (file - 2));
    if (rank - 1 >= 0 && file + 2 < 8) attacks |= 1ULL << ((rank - 1) * 8 + (file + 2));
    if (rank - 1 >= 0 && file - 2 >= 0) attacks |= 1ULL << ((rank - 1) * 8 + (file - 2));

    return attacks;
}

void Knight::initKnightAttacks()
{
    for (int square = 0; square < 64; square++)
        KnightAttackTable[square] = attacksSlow(square);
}

Bitboard Knight::attacks(int square)
{
    return KnightAttackTable[square];
}