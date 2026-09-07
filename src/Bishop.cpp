#include "Bishop.h"

Bitboard Bishop::BishopAttackTable[64][4096];
Bitboard Bishop::BishopMagics[64];
int      Bishop::BishopRelevantBits[64];

Bitboard Bishop::BishopMask(int square)
{
    const int rank = square / 8;
    const int file = square % 8;

    Bitboard mask = 0ULL;

    for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++)
        mask |= 1ULL << (r * 8 + f);

    for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--)
        mask |= 1ULL << (r * 8 + f);

    for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++)
        mask |= 1ULL << (r * 8 + f);

    for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--)
        mask |= 1ULL << (r * 8 + f);

    return mask;
}

Bitboard Bishop::attacksSlow(int square, Bitboard occupied)
{
    const int rank = square / 8;
    const int file = square % 8;

    Bitboard attacks = 0ULL;

    for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++)
    {
        const int sq = r * 8 + f;
        attacks |= 1ULL << sq;
        if (occupied & (1ULL << sq)) break;
    }

    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
    {
        const int sq = r * 8 + f;
        attacks |= 1ULL << sq;
        if (occupied & (1ULL << sq)) break;
    }

    for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++)
    {
        const int sq = r * 8 + f;
        attacks |= 1ULL << sq;
        if (occupied & (1ULL << sq)) break;
    }

    for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--)
    {
        const int sq = r * 8 + f;
        attacks |= 1ULL << sq;
        if (occupied & (1ULL << sq)) break;
    }

    return attacks;
}

void Bishop::initBishopAttacks()
{
    for (int square = 0; square < 64; square++)
    {
        const Bitboard mask = BishopMask(square);
        const int relevantBits = BitboardUtils::popcount(mask);

        BishopRelevantBits[square] = relevantBits;
        BishopMagics[square] = BitboardUtils::findMagic(square, relevantBits, mask, attacksSlow);

        const int possibleOccupancies = 1 << relevantBits;

        for (int index = 0; index < possibleOccupancies; index++)
        {
            const Bitboard occupancy = BitboardUtils::setOccupancy(index, relevantBits, mask);
            const Bitboard attack = attacksSlow(square, occupancy);

            const int magicIndex =
                static_cast<int>((occupancy * BishopMagics[square]) >> (64 - relevantBits));

            BishopAttackTable[square][magicIndex] = attack;
        }
    }
}

Bitboard Bishop::attacks(int square, Bitboard occupied)
{
    const Bitboard mask = BishopMask(square);
    occupied &= mask;

    const int index =
        static_cast<int>((occupied * BishopMagics[square]) >> (64 - BishopRelevantBits[square]));

    return BishopAttackTable[square][index];
}