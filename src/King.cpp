#include "King.h"

Bitboard King::KingAttackTable[64];

Bitboard King::attacksSlow(int square)
{
    Bitboard attacks = 0ULL;
    const int rank = square / 8;
    const int file = square % 8;

    if (rank + 1 < 8 && file + 1 < 8) attacks |= 1ULL << ((rank + 1) * 8 + (file + 1));
    if (rank - 1 >= 0 && file + 1 < 8) attacks |= 1ULL << ((rank - 1) * 8 + (file + 1));
    if (rank + 1 < 8 && file - 1 >= 0) attacks |= 1ULL << ((rank + 1) * 8 + (file - 1));
    if (rank - 1 >= 0 && file - 1 >= 0) attacks |= 1ULL << ((rank - 1) * 8 + (file - 1));

    if (rank + 1 < 8) attacks |= 1ULL << ((rank + 1) * 8 + file);
    if (rank - 1 >= 0) attacks |= 1ULL << ((rank - 1) * 8 + file);
    if (file + 1 < 8) attacks |= 1ULL << (rank * 8 + (file + 1));
    if (file - 1 >= 0) attacks |= 1ULL << (rank * 8 + (file - 1));

    return attacks;
}

void King::initKingAttacks()
{
    for (int square = 0; square < 64; square++)
        KingAttackTable[square] = attacksSlow(square);
}

Bitboard King::attacks(int square)
{
    return KingAttackTable[square];
}